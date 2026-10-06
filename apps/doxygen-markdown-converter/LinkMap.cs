using System.Text;
using HtmlAgilityPack;

namespace doxygen_markdown_converter;

/// <summary>
/// Maps Doxygen HTML file names to GitHub-wiki page slugs and rewrites links.
///
/// A GitHub wiki is a flat namespace: every page is a single <c>*.md</c> file in
/// the repo root and the page's display name (shown in the auto "Pages" panel
/// and as the page header) is derived from the <em>file name</em> with dashes
/// turned into spaces. There is no title metadata.
///
/// So to give pages real titles we name each file after its Doxygen page title,
/// slugified (e.g. "Console functions" -> <c>Console-functions.md</c>, which the
/// wiki shows as "Console functions"). The index page keeps the explicit name
/// supplied on the command line. Slugs are de-duplicated so collisions (several
/// Doxygen index pages share a title) stay unique.
/// </summary>
internal sealed class LinkMap
{
    private const string IndexHtml = "index.html";

    private readonly string _indexSlug;
    private readonly Dictionary<string, string> _htmlToSlug =
        new(StringComparer.OrdinalIgnoreCase);

    /// <param name="indexMarkdownName">Markdown file name index.html maps to.</param>
    /// <param name="htmlTitles">Map of html file name -> raw page title.</param>
    public LinkMap(string indexMarkdownName, IReadOnlyDictionary<string, string> htmlTitles)
    {
        _indexSlug = Path.GetFileNameWithoutExtension(indexMarkdownName);

        var used = new HashSet<string>(StringComparer.OrdinalIgnoreCase) { _indexSlug };
        _htmlToSlug[IndexHtml] = _indexSlug;

        // Deterministic ordering so de-dup suffixes are stable across runs.
        foreach (string html in htmlTitles.Keys.OrderBy(k => k, StringComparer.Ordinal))
        {
            if (html.Equals(IndexHtml, StringComparison.OrdinalIgnoreCase))
                continue;

            string title = htmlTitles[html];
            string baseSlug = Slugify(title);
            if (baseSlug.Length == 0)
                baseSlug = Slugify(Path.GetFileNameWithoutExtension(html));

            string slug = baseSlug;
            for (int n = 2; used.Contains(slug); n++)
                slug = $"{baseSlug}-{n}";

            used.Add(slug);
            _htmlToSlug[html] = slug;
        }
    }

    /// <summary>Output markdown file name for a source html file.</summary>
    public string MarkdownFileFor(string htmlFileName) => SlugFor(htmlFileName) + ".md";

    /// <summary>Wiki slug (page name without extension) for a source html file.</summary>
    public string SlugFor(string htmlFileName) =>
        _htmlToSlug.TryGetValue(htmlFileName, out string? slug)
            ? slug
            : Slugify(Path.GetFileNameWithoutExtension(htmlFileName));

    /// <summary>
    /// Rewrites a Doxygen href (found on <paramref name="currentHtmlFile"/>) into
    /// a wiki link target. Returns <c>null</c> to drop the link.
    /// </summary>
    public string? RewriteHref(string href, string currentHtmlFile)
    {
        if (string.IsNullOrWhiteSpace(href))
            return null;

        href = href.Trim();

        if (href.StartsWith("http://") || href.StartsWith("https://") ||
            href.StartsWith("mailto:") || href.StartsWith("ftp://") ||
            href.StartsWith("//"))
            return href;

        // Pure in-page anchor: the converter emits matching <a name="..."></a>.
        if (href.StartsWith('#'))
            return href;

        string file = href;
        string anchor = string.Empty;
        int hash = href.IndexOf('#');
        if (hash >= 0)
        {
            file = href[..hash];
            anchor = href[(hash + 1)..];
        }

        if (file.Length == 0)
            return href;

        if (file.EndsWith(".html", StringComparison.OrdinalIgnoreCase))
        {
            // Link into the current page -> prefer a plain in-page anchor.
            if (file.Equals(currentHtmlFile, StringComparison.OrdinalIgnoreCase) &&
                anchor.Length > 0)
                return $"#{anchor}";

            string slug = SlugFor(file);
            return anchor.Length > 0 ? $"{slug}#{anchor}" : slug;
        }

        // Images / other resources keep their relative path.
        return href;
    }

    /// <summary>
    /// Turns a page title into a GitHub-wiki-safe slug: ASCII alphanumerics are
    /// kept, every other run of characters becomes a single dash. The wiki then
    /// renders the dashes back as spaces in the page title.
    /// </summary>
    public static string Slugify(string title)
    {
        string t = HtmlEntity.DeEntitize(title);
        var sb = new StringBuilder(t.Length);
        bool lastDash = false;
        foreach (char c in t)
        {
            bool keep = c is (>= 'a' and <= 'z') or (>= 'A' and <= 'Z') or (>= '0' and <= '9');
            if (keep)
            {
                sb.Append(c);
                lastDash = false;
            }
            else if (!lastDash)
            {
                sb.Append('-');
                lastDash = true;
            }
        }
        return sb.ToString().Trim('-');
    }
}
