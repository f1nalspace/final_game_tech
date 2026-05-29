using System.Text;
using System.Text.RegularExpressions;
using HtmlAgilityPack;

namespace doxygen_markdown_converter;

/// <summary>
/// Converts the body of a single Doxygen HTML page into GitHub-wiki Markdown.
///
/// The renderer walks the DOM of the <c>&lt;div class="contents"&gt;</c> block
/// and maps Doxygen's structural markup (member tables, function prototypes,
/// parameter lists, note/warning sections, code fragments, ...) to Markdown.
///
/// Internal links are rewritten through <see cref="LinkMap"/> so they point to
/// the correct wiki page, and Doxygen anchors are preserved as raw
/// <c>&lt;a name="..."&gt;&lt;/a&gt;</c> markers (which GitHub keeps) so in-page
/// <c>#anchor</c> links keep working after conversion.
/// </summary>
internal sealed class DoxygenMarkdownConverter
{
    private readonly LinkMap _links;
    private string _currentHtml = string.Empty;

    public DoxygenMarkdownConverter(LinkMap links) => _links = links;

    /// <summary>State threaded through the recursive renderer.</summary>
    private struct Ctx
    {
        public int ListDepth;
        public bool InTableCell;
        public readonly Ctx Deeper() => new() { ListDepth = ListDepth + 1, InTableCell = InTableCell };
        public readonly Ctx InCell() => new() { ListDepth = ListDepth, InTableCell = true };
    }

    public string ConvertFile(string path)
    {
        _currentHtml = Path.GetFileName(path);
        var doc = new HtmlDocument
        {
            OptionDefaultStreamEncoding = Encoding.UTF8,
        };
        doc.Load(path, Encoding.UTF8);

        var sb = new StringBuilder();

        string? title = ExtractTitle(doc);
        if (!string.IsNullOrEmpty(title))
            sb.Append("# ").Append(title).Append("\n\n");

        HtmlNode? content =
            doc.DocumentNode.SelectSingleNode("//div[@class='contents']")
            ?? doc.DocumentNode.SelectSingleNode("//body");

        if (content != null)
            RenderChildren(content, sb, new Ctx());

        return Normalize(sb.ToString());
    }

    private static string? ExtractTitle(HtmlDocument doc)
    {
        HtmlNode? t = doc.DocumentNode
            .SelectSingleNode("//div[@class='headertitle']//div[@class='title']");
        if (t != null)
        {
            // The title div may contain a nested <div class="ingroups"> link; keep
            // only the title's own direct text (e.g. "fplVec2 Struct Reference").
            string raw = string.Concat(t.ChildNodes
                .Where(n => n.NodeType == HtmlNodeType.Text)
                .Select(n => n.InnerText));
            string s = CollapseWs(Decode(raw)).Trim();
            if (s.Length > 0)
                return s;
        }

        HtmlNode? titleTag = doc.DocumentNode.SelectSingleNode("//title");
        if (titleTag != null)
        {
            string s = CollapseWs(Decode(titleTag.InnerText)).Trim();
            int colon = s.IndexOf(':');
            if (colon >= 0 && colon + 1 < s.Length)
                s = s[(colon + 1)..].Trim();
            return s.Length > 0 ? s : null;
        }
        return null;
    }

    // ---- Block-level rendering -------------------------------------------------

    private void RenderChildren(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        foreach (HtmlNode child in node.ChildNodes)
            RenderNode(child, sb, ctx);
    }

    private void RenderNode(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        if (node.NodeType == HtmlNodeType.Comment)
            return;

        if (node.NodeType == HtmlNodeType.Text)
        {
            // Bare text at block level (e.g. the "generated from..." note) is a
            // paragraph in its own right.
            string txt = EscapeText(node.InnerText, ctx).Trim();
            if (txt.Length > 0)
                sb.Append(txt).Append("\n\n");
            return;
        }

        if (node.NodeType != HtmlNodeType.Element)
            return;

        string cls = node.GetAttributeValue("class", string.Empty);

        switch (node.Name)
        {
            case "h1": case "h2": case "h3":
            case "h4": case "h5": case "h6":
                RenderHeading(node, sb, ctx);
                break;

            case "p":
                RenderMixed(node, sb, ctx);
                break;

            case "ul": RenderList(node, sb, ctx, ordered: false); break;
            case "ol": RenderList(node, sb, ctx, ordered: true); break;

            case "table": RenderTable(node, sb, ctx); break;

            case "dl": RenderDl(node, sb, ctx); break;

            case "pre": RenderCodeText(node.InnerText, sb); break;

            case "blockquote":
            {
                var inner = new StringBuilder();
                RenderChildren(node, inner, ctx);
                foreach (string line in inner.ToString().TrimEnd().Split('\n'))
                    sb.Append("> ").Append(line).Append('\n');
                sb.Append('\n');
                break;
            }

            case "hr":
                if (!cls.Contains("footer"))
                    sb.Append("\n---\n\n");
                break;

            case "div":
                if (cls.Contains("fragment")) RenderFragment(node, sb);
                else if (cls.Contains("memproto")) RenderMemProto(node, sb);
                else RenderChildren(node, sb, ctx); // textblock, memitem, memdoc, toc, ...
                break;

            case "address": // doxygen footer
                break;

            default:
            {
                // Inline element appearing at block level (anchors, stray text).
                var tmp = new StringBuilder();
                InlineNode(node, tmp, ctx);
                string s = tmp.ToString();
                if (s.Contains("<a name"))
                    sb.Append(s).Append('\n');
                else if (s.Trim().Length > 0)
                    sb.Append(s).Append('\n');
                break;
            }
        }
    }

    /// <summary>
    /// Renders mixed inline/block content (e.g. a &lt;p&gt; that contains a
    /// nested list): inline runs are flushed as paragraphs, block children are
    /// rendered in place.
    /// </summary>
    private void RenderMixed(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        var inline = new StringBuilder();
        void Flush()
        {
            string t = CollapseWs(inline.ToString()).Trim();
            if (t.Length > 0)
                sb.Append(t).Append("\n\n");
            inline.Clear();
        }

        foreach (HtmlNode child in node.ChildNodes)
        {
            if (child.NodeType == HtmlNodeType.Element && IsBlockElement(child))
            {
                Flush();
                RenderNode(child, sb, ctx);
            }
            else
            {
                InlineNode(child, inline, ctx);
            }
        }
        Flush();
    }

    private static bool IsBlockElement(HtmlNode node) => node.Name switch
    {
        "ul" or "ol" or "table" or "dl" or "pre" or "blockquote" or "hr" or "div"
            or "h1" or "h2" or "h3" or "h4" or "h5" or "h6" => true,
        _ => false,
    };

    private void RenderHeading(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        int level = node.Name[1] - '0';
        string text = CollapseWs(RenderInline(node, ctx)).Trim();
        if (text.Length == 0)
            return;
        sb.Append(new string('#', level)).Append(' ').Append(text).Append("\n\n");
    }

    private void RenderList(HtmlNode node, StringBuilder sb, Ctx ctx, bool ordered)
    {
        string indent = new(' ', ctx.ListDepth * 2);
        int index = 1;
        foreach (HtmlNode li in node.ChildNodes)
        {
            if (li.NodeType != HtmlNodeType.Element || li.Name != "li")
                continue;

            var inlineBuf = new StringBuilder();
            var nested = new StringBuilder();
            foreach (HtmlNode c in li.ChildNodes)
            {
                if (c.NodeType == HtmlNodeType.Element && (c.Name == "ul" || c.Name == "ol"))
                    RenderList(c, nested, ctx.Deeper(), c.Name == "ol");
                else if (c.NodeType == HtmlNodeType.Element && c.Name == "p")
                    inlineBuf.Append(RenderInline(c, ctx)).Append(' ');
                else
                    InlineNode(c, inlineBuf, ctx);
            }

            string text = CollapseWs(inlineBuf.ToString()).Trim();
            string marker = ordered ? $"{index}." : "-";
            sb.Append(indent).Append(marker).Append(' ').Append(text).Append('\n');
            sb.Append(nested);
            index++;
        }
        if (ctx.ListDepth == 0)
            sb.Append('\n');
    }

    private void RenderDl(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        string cls = node.GetAttributeValue("class", string.Empty);
        bool section = cls.Contains("section");

        foreach (HtmlNode child in node.ChildNodes)
        {
            if (child.NodeType != HtmlNodeType.Element)
                continue;

            if (child.Name == "dt")
            {
                string label = CollapseWs(RenderInline(child, ctx)).Trim();
                if (label.Length > 0)
                {
                    if (section)
                        sb.Append("> **").Append(label).Append(":** ");
                    else
                        sb.Append("**").Append(label).Append("**\n\n");
                }
            }
            else if (child.Name == "dd")
            {
                if (section)
                {
                    // Note / Warning / Returns / See also … rendered as a quote.
                    var inner = new StringBuilder();
                    RenderChildren(child, inner, ctx);
                    string body = inner.ToString().Trim();
                    string[] lines = body.Split('\n');
                    sb.Append(lines.Length > 0 ? lines[0] : string.Empty).Append('\n');
                    for (int i = 1; i < lines.Length; i++)
                        sb.Append("> ").Append(lines[i]).Append('\n');
                    sb.Append('\n');
                }
                else
                {
                    RenderChildren(child, sb, ctx);
                }
            }
        }
    }

    // ---- Tables ----------------------------------------------------------------

    private void RenderTable(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        string cls = node.GetAttributeValue("class", string.Empty);
        if (cls.Contains("memberdecls")) RenderMemberDecls(node, sb, ctx);
        else if (cls.Contains("memname")) RenderSignatureTable(node, sb);
        else if (cls.Contains("params")) RenderParamsTable(node, sb, ctx);
        else if (cls.Contains("fieldtable")) RenderFieldTable(node, sb, ctx);
        else if (cls.Contains("directory")) RenderDirectoryTable(node, sb, ctx);
        else RenderGenericTable(node, sb, ctx);
    }

    private void RenderMemberDecls(HtmlNode table, StringBuilder sb, Ctx ctx)
    {
        Ctx cell = ctx.InCell();
        bool headerEmitted = false;

        void EnsureHeader()
        {
            if (headerEmitted) return;
            sb.Append("\n| Type | Name |\n|---|---|\n");
            headerEmitted = true;
        }

        string? pendingType = null;
        string? pendingName = null;

        void Flush(string? desc)
        {
            if (pendingName == null) return;
            EnsureHeader();
            string name = pendingName;
            if (!string.IsNullOrEmpty(desc))
                name += "<br>" + desc;
            sb.Append("| ").Append(pendingType ?? string.Empty).Append(" | ")
              .Append(name).Append(" |\n");
            pendingType = null;
            pendingName = null;
        }

        foreach (HtmlNode tr in table.SelectNodes(".//tr") ?? Enumerable.Empty<HtmlNode>())
        {
            string rowCls = tr.GetAttributeValue("class", string.Empty);

            if (rowCls.Contains("heading"))
            {
                Flush(null);
                headerEmitted = false;
                HtmlNode? h = tr.SelectSingleNode(".//h2");
                if (h != null)
                {
                    string ht = CollapseWs(RenderInline(h, ctx)).Trim();
                    if (ht.Length > 0)
                        sb.Append("\n### ").Append(ht).Append("\n");
                }
                continue;
            }

            if (rowCls.Contains("memdesc"))
            {
                HtmlNode? d = tr.SelectSingleNode(".//td[contains(@class,'mdescRight')]");
                Flush(Cell(d, cell));
                continue;
            }

            if (rowCls.Contains("memitem"))
            {
                Flush(null); // a previous memitem without its own memdesc
                HtmlNode? left = tr.SelectSingleNode(".//td[contains(@class,'memItemLeft')]");
                HtmlNode? right = tr.SelectSingleNode(".//td[contains(@class,'memItemRight')]");
                pendingType = Cell(left, cell);
                pendingName = Cell(right, cell);
            }
        }

        Flush(null);
        sb.Append('\n');
    }

    private static void RenderSignatureTable(HtmlNode table, StringBuilder sb)
    {
        string sig = CollapseWs(Decode(table.InnerText)).Trim();
        if (sig.Length == 0)
            return;
        sb.Append("```c\n").Append(sig).Append("\n```\n\n");
    }

    private void RenderMemProto(HtmlNode div, StringBuilder sb)
    {
        HtmlNode? table = div.SelectSingleNode(".//table[contains(@class,'memname')]");
        if (table != null)
            RenderSignatureTable(table, sb);
        else
        {
            string sig = CollapseWs(Decode(div.InnerText)).Trim();
            if (sig.Length > 0)
                sb.Append("```c\n").Append(sig).Append("\n```\n\n");
        }
    }

    private void RenderParamsTable(HtmlNode table, StringBuilder sb, Ctx ctx)
    {
        Ctx cell = ctx.InCell();
        var rows = new List<(string dir, string name, string desc)>();
        bool anyDir = false;

        foreach (HtmlNode tr in table.SelectNodes(".//tr") ?? Enumerable.Empty<HtmlNode>())
        {
            HtmlNode? dir = tr.SelectSingleNode(".//td[contains(@class,'paramdir')]");
            HtmlNode? name = tr.SelectSingleNode(".//td[contains(@class,'paramname')]");
            HtmlNode? desc = tr.SelectSingleNode("./td[not(@class) or (not(contains(@class,'paramdir')) and not(contains(@class,'paramname')))]");

            string d = Cell(dir, cell);
            string n = Cell(name, cell);
            string ds = Cell(desc, cell);
            if (d.Length > 0) anyDir = true;
            if (n.Length == 0 && ds.Length == 0 && d.Length == 0) continue;
            rows.Add((d, n, ds));
        }
        if (rows.Count == 0)
            return;

        if (anyDir)
        {
            sb.Append("\n| Direction | Parameter | Description |\n|---|---|---|\n");
            foreach (var (d, n, ds) in rows)
                sb.Append("| ").Append(d).Append(" | ").Append(n).Append(" | ").Append(ds).Append(" |\n");
        }
        else
        {
            sb.Append("\n| Parameter | Description |\n|---|---|\n");
            foreach (var (_, n, ds) in rows)
                sb.Append("| ").Append(n).Append(" | ").Append(ds).Append(" |\n");
        }
        sb.Append('\n');
    }

    private void RenderFieldTable(HtmlNode table, StringBuilder sb, Ctx ctx)
    {
        Ctx cell = ctx.InCell();
        sb.Append("\n| Name | Description |\n|---|---|\n");
        foreach (HtmlNode tr in table.SelectNodes(".//tr") ?? Enumerable.Empty<HtmlNode>())
        {
            HtmlNode? name = tr.SelectSingleNode(".//td[contains(@class,'fieldname')]");
            HtmlNode? doc = tr.SelectSingleNode(".//td[contains(@class,'fielddoc')]");
            string n = Cell(name, cell);
            string d = Cell(doc, cell);
            if (n.Length == 0 && d.Length == 0) continue;
            sb.Append("| ").Append(n).Append(" | ").Append(d).Append(" |\n");
        }
        sb.Append('\n');
    }

    private void RenderDirectoryTable(HtmlNode table, StringBuilder sb, Ctx ctx)
    {
        Ctx cell = ctx.InCell();
        sb.Append("\n| Name | Description |\n|---|---|\n");
        foreach (HtmlNode tr in table.SelectNodes(".//tr") ?? Enumerable.Empty<HtmlNode>())
        {
            HtmlNode? entry = tr.SelectSingleNode(".//td[contains(@class,'entry')]");
            HtmlNode? desc = tr.SelectSingleNode(".//td[contains(@class,'desc')]");
            // Drop the tree-indent / icon spans; keep just the link text.
            string n = Cell(entry, cell);
            string d = Cell(desc, cell);
            if (n.Length == 0 && d.Length == 0) continue;
            sb.Append("| ").Append(n).Append(" | ").Append(d).Append(" |\n");
        }
        sb.Append('\n');
    }

    private void RenderGenericTable(HtmlNode table, StringBuilder sb, Ctx ctx)
    {
        Ctx cell = ctx.InCell();
        var rows = new List<List<string>>();
        int cols = 0;
        foreach (HtmlNode tr in table.SelectNodes(".//tr") ?? Enumerable.Empty<HtmlNode>())
        {
            var cells = new List<string>();
            foreach (HtmlNode td in tr.ChildNodes)
            {
                if (td.NodeType != HtmlNodeType.Element || (td.Name != "td" && td.Name != "th"))
                    continue;
                cells.Add(Cell(td, cell));
            }
            if (cells.Count == 0)
                continue;
            cols = Math.Max(cols, cells.Count);
            rows.Add(cells);
        }
        if (rows.Count == 0 || cols == 0)
            return;

        void Pad(List<string> r) { while (r.Count < cols) r.Add(string.Empty); }

        sb.Append('\n');
        Pad(rows[0]);
        sb.Append("| ").Append(string.Join(" | ", rows[0])).Append(" |\n");
        sb.Append('|').Append(string.Concat(Enumerable.Repeat("---|", cols))).Append('\n');
        for (int i = 1; i < rows.Count; i++)
        {
            Pad(rows[i]);
            sb.Append("| ").Append(string.Join(" | ", rows[i])).Append(" |\n");
        }
        sb.Append('\n');
    }

    // ---- Code fragments --------------------------------------------------------

    private static void RenderFragment(HtmlNode div, StringBuilder sb)
    {
        var lines = new List<string>();
        HtmlNodeCollection? lineNodes = div.SelectNodes(".//div[contains(@class,'line')]");
        if (lineNodes != null)
        {
            foreach (HtmlNode line in lineNodes)
            {
                var b = new StringBuilder();
                foreach (HtmlNode c in line.ChildNodes)
                {
                    if (c.NodeType == HtmlNodeType.Element)
                    {
                        if (c.Name == "a") continue; // line-number anchor (empty)
                        string ccls = c.GetAttributeValue("class", string.Empty);
                        if (c.Name == "span" && ccls.Contains("lineno")) continue;
                    }
                    b.Append(Decode(c.InnerText));
                }
                lines.Add(b.ToString().TrimEnd());
            }
        }
        else
        {
            lines.Add(Decode(div.InnerText));
        }

        sb.Append("```c\n");
        sb.Append(string.Join("\n", lines));
        sb.Append("\n```\n\n");
    }

    private static void RenderCodeText(string raw, StringBuilder sb)
    {
        string code = Decode(raw).Replace("\r\n", "\n").TrimEnd();
        sb.Append("```c\n").Append(code).Append("\n```\n\n");
    }

    // ---- Inline rendering ------------------------------------------------------

    private string RenderInline(HtmlNode node, Ctx ctx)
    {
        var sb = new StringBuilder();
        foreach (HtmlNode child in node.ChildNodes)
            InlineNode(child, sb, ctx);
        return sb.ToString();
    }

    private void InlineNode(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        if (node.NodeType == HtmlNodeType.Comment)
            return;
        if (node.NodeType == HtmlNodeType.Text)
        {
            sb.Append(EscapeText(node.InnerText, ctx));
            return;
        }
        if (node.NodeType != HtmlNodeType.Element)
            return;

        string cls = node.GetAttributeValue("class", string.Empty);

        switch (node.Name)
        {
            case "br":
                sb.Append(ctx.InTableCell ? "<br>" : "  \n");
                break;

            case "b": case "strong":
            {
                string inner = RenderInline(node, ctx).Trim();
                if (inner.Length > 0) sb.Append("**").Append(inner).Append("**");
                break;
            }

            case "i": case "em":
            {
                string inner = RenderInline(node, ctx).Trim();
                if (inner.Length > 0) sb.Append('*').Append(inner).Append('*');
                break;
            }

            case "code": case "tt": case "kbd": case "samp": case "computeroutput":
                AppendInlineCode(node.InnerText, sb);
                break;

            case "a":
                AppendAnchor(node, sb, ctx);
                break;

            case "img":
            {
                string src = node.GetAttributeValue("src", string.Empty);
                if (src.Length == 0 || src.EndsWith("doxygen.svg")) break;
                string alt = node.GetAttributeValue("alt", string.Empty);
                string? rew = _links.RewriteHref(src, _currentHtml) ?? src;
                sb.Append("![").Append(EscapeBracket(alt)).Append("](").Append(rew).Append(')');
                break;
            }

            case "span":
                if (cls.Contains("permalink") || cls.Contains("lineno"))
                    break;
                sb.Append(RenderInline(node, ctx));
                break;

            case "ul": case "ol":
                // Lists nested inside inline content: fall back to block rendering.
                RenderList(node, sb, ctx.Deeper(), node.Name == "ol");
                break;

            default:
                sb.Append(RenderInline(node, ctx));
                break;
        }
    }

    private void AppendAnchor(HtmlNode node, StringBuilder sb, Ctx ctx)
    {
        string text = RenderInline(node, ctx).Trim();
        text = StripPermalinkGlyphs(text).Trim();
        string href = node.GetAttributeValue("href", string.Empty);

        if (text.Length == 0)
        {
            // Pure anchor target: preserve as an HTML anchor GitHub keeps.
            string anchorName =
                node.GetAttributeValue("name", null!) ??
                node.GetAttributeValue("id", null!);
            if (!string.IsNullOrEmpty(anchorName))
                sb.Append("<a name=\"").Append(anchorName).Append("\"></a>");
            return;
        }

        if (string.IsNullOrEmpty(href))
        {
            sb.Append(text);
            return;
        }

        string? target = _links.RewriteHref(href, _currentHtml);
        if (string.IsNullOrEmpty(target))
        {
            sb.Append(text);
            return;
        }
        sb.Append('[').Append(text).Append("](").Append(target).Append(')');
    }

    private static void AppendInlineCode(string raw, StringBuilder sb)
    {
        string code = CollapseWs(Decode(raw)).Trim();
        if (code.Length == 0)
            return;
        if (code.Contains('`'))
            sb.Append("`` ").Append(code).Append(" ``");
        else
            sb.Append('`').Append(code).Append('`');
    }

    // ---- Text / escaping helpers ----------------------------------------------

    private static string Decode(string s) =>
        HtmlEntity.DeEntitize(s).Replace('\u00A0', ' ');

    private static string CollapseWs(string s) =>
        Regex.Replace(s, @"[ \t\r\n\f]+", " ");

    private static string StripPermalinkGlyphs(string s) =>
        s.Replace("◆", string.Empty)  // ◆ black diamond (permalink marker)
         .Replace("♦", string.Empty)  // ♦
         .Replace("¶", string.Empty); // ¶

    private static string EscapeBracket(string s) =>
        s.Replace("[", "\\[").Replace("]", "\\]");

    /// <summary>Renders a table cell, stripping leading/trailing line breaks.</summary>
    private string Cell(HtmlNode? node, Ctx cell) =>
        node != null ? CleanCell(CollapseWs(RenderInline(node, cell))) : string.Empty;

    private static string CleanCell(string s)
    {
        s = s.Trim();
        s = Regex.Replace(s, @"(\s*<br>\s*)+$", string.Empty);
        s = Regex.Replace(s, @"^(\s*<br>\s*)+", string.Empty);
        return s.Trim();
    }

    private static string EscapeText(string raw, Ctx ctx)
    {
        string s = Decode(raw);
        s = CollapseWs(s);
        s = StripPermalinkGlyphs(s);

        s = s.Replace("\\", "\\\\");
        s = s.Replace("`", "\\`");
        s = s.Replace("*", "\\*");
        s = s.Replace("[", "\\[").Replace("]", "\\]");

        // HTML-encode markup characters (& first, so we don't double-encode).
        s = s.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");

        if (ctx.InTableCell)
            s = s.Replace("|", "\\|");

        return s;
    }

    private static string Normalize(string text)
    {
        text = text.Replace("\r\n", "\n").Replace('\u00A0', ' ');
        text = StripPermalinkGlyphs(text);
        // Collapse 3+ blank lines into a single blank line.
        text = Regex.Replace(text, "\n{3,}", "\n\n");
        return text.TrimEnd() + "\n";
    }
}
