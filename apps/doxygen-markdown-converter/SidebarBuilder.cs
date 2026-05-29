using System.Text;
using System.Text.RegularExpressions;
using HtmlAgilityPack;

namespace doxygen_markdown_converter;

/// <summary>
/// Builds a <c>_Sidebar.md</c> that mirrors Doxygen's navigation as a nested
/// list. GitHub's auto "Pages" panel is always flat; <c>_Sidebar.md</c> is the
/// only way to present a real hierarchy, and it renders on every wiki page.
///
/// The tree is reconstructed from Doxygen's listing pages:
/// <list type="bullet">
///   <item><b>Pages</b> — the related-page links on <c>index.html</c>.</item>
///   <item><b>Topics</b> — <c>topics.html</c> (the module/group tree).</item>
///   <item><b>Data Structures</b> — <c>annotated.html</c>.</item>
///   <item><b>Files</b> — <c>files.html</c>.</item>
/// </list>
/// In Doxygen's directory tables the row <c>id="row_0_1_..."</c> encodes the
/// tree depth (one segment per level), which is used for indentation.
/// </summary>
internal sealed class SidebarBuilder
{
    private readonly string _sourceDir;
    private readonly LinkMap _links;

    public SidebarBuilder(string sourceDir, LinkMap links)
    {
        _sourceDir = sourceDir;
        _links = links;
    }

    public string Build(string indexTitle)
    {
        var sb = new StringBuilder();

        string indexSlug = _links.SlugFor("index.html");
        sb.Append("**[").Append(EscapeLabel(indexTitle)).Append("](").Append(indexSlug).Append(")**\n\n");

        EmitSection(sb, "Pages", ParseIndexPages());
        EmitSection(sb, "Topics", ParseDirectory("topics.html"));
        EmitSection(sb, "Data Structures", ParseDirectory("annotated.html"));
        EmitSection(sb, "Files", ParseDirectory("files.html"));

        return sb.ToString().TrimEnd() + "\n";
    }

    private readonly record struct Entry(int Depth, string Label, string Href);

    private void EmitSection(StringBuilder sb, string heading, List<Entry> entries)
    {
        if (entries.Count == 0)
            return;

        sb.Append("### ").Append(heading).Append("\n\n");
        foreach (Entry e in entries)
        {
            string file = e.Href;
            string anchor = string.Empty;
            int hash = e.Href.IndexOf('#');
            if (hash >= 0)
            {
                file = e.Href[..hash];
                anchor = e.Href[(hash + 1)..];
            }

            string target = _links.SlugFor(file);
            if (anchor.Length > 0)
                target += "#" + anchor;

            sb.Append(new string(' ', e.Depth * 2))
              .Append("- [").Append(EscapeLabel(e.Label)).Append("](").Append(target).Append(")\n");
        }
        sb.Append('\n');
    }

    /// <summary>Reads a Doxygen directory listing (topics/annotated/files).</summary>
    private List<Entry> ParseDirectory(string fileName)
    {
        var result = new List<Entry>();
        string path = Path.Combine(_sourceDir, fileName);
        if (!File.Exists(path))
            return result;

        HtmlDocument doc = Load(path);
        HtmlNodeCollection? rows =
            doc.DocumentNode.SelectNodes("//table[contains(@class,'directory')]//tr");
        if (rows == null)
            return result;

        foreach (HtmlNode tr in rows)
        {
            string id = tr.GetAttributeValue("id", string.Empty);
            if (!id.StartsWith("row_"))
                continue;

            int levels = id.Split('_', StringSplitOptions.RemoveEmptyEntries)
                           .Count(p => int.TryParse(p, out _));
            int depth = Math.Max(0, levels - 1);

            HtmlNode? a = tr.SelectSingleNode(".//td[contains(@class,'entry')]//a[@href]");
            if (a == null)
                continue;

            string href = a.GetAttributeValue("href", string.Empty);
            string label = CleanText(a.InnerText);
            if (href.Length == 0 || label.Length == 0)
                continue;

            result.Add(new Entry(depth, label, href));
        }
        return result;
    }

    /// <summary>Top-level related pages, taken from the links on index.html.</summary>
    private List<Entry> ParseIndexPages()
    {
        var result = new List<Entry>();
        string path = Path.Combine(_sourceDir, "index.html");
        if (!File.Exists(path))
            return result;

        HtmlDocument doc = Load(path);
        HtmlNode? content = doc.DocumentNode.SelectSingleNode("//div[@class='contents']");
        // Only the curated "How to use this documentation?" list items, not the
        // stray page links scattered through the prose paragraphs above it.
        HtmlNodeCollection? anchors = content?.SelectNodes(".//li/a[@href]");
        if (anchors == null)
            return result;

        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (HtmlNode a in anchors)
        {
            string href = a.GetAttributeValue("href", string.Empty);
            string file = href.Split('#')[0];
            if (!Regex.IsMatch(file, @"^page_.+\.html$", RegexOptions.IgnoreCase))
                continue;
            if (!seen.Add(file))
                continue;

            string label = CleanText(a.InnerText);
            if (label.Length > 0)
                result.Add(new Entry(0, label, file));
        }
        return result;
    }

    private static HtmlDocument Load(string path)
    {
        var doc = new HtmlDocument { OptionDefaultStreamEncoding = Encoding.UTF8 };
        doc.Load(path, Encoding.UTF8);
        return doc;
    }

    private static string CleanText(string s) =>
        Regex.Replace(HtmlEntity.DeEntitize(s).Replace(' ', ' '), @"\s+", " ").Trim();

    private static string EscapeLabel(string s) =>
        s.Replace("[", "\\[").Replace("]", "\\]");
}
