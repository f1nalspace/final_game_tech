using System.Text;

namespace doxygen_markdown_converter;

/// <summary>
/// Maps Doxygen HTML file names to GitHub-wiki page locations and rewrites
/// links between them.
///
/// Layout produced:
/// <code>
///   &lt;wiki-root&gt;/FPL-Documentation.md          (from index.html)
///   &lt;wiki-root&gt;/FPL-Documentation/&lt;page&gt;.md   (every other page)
/// </code>
/// The sub-folder is named after the index page (without extension). Links are
/// emitted as <em>relative</em> paths computed between the two files, so they
/// resolve correctly regardless of which folder a page lives in (root index ->
/// folder page, folder page -> sibling, folder page -> root index).
/// </summary>
internal sealed class LinkMap
{
    private const string IndexHtml = "index.html";

    private readonly string _indexSlug;   // e.g. "FPL-Documentation"
    private readonly string _folder;      // sub-folder for non-index pages
    private readonly HashSet<string> _known = new(StringComparer.OrdinalIgnoreCase);

    public LinkMap(string indexMarkdownName)
    {
        _indexSlug = Path.GetFileNameWithoutExtension(indexMarkdownName);
        _folder = _indexSlug;
        _known.Add(IndexHtml);
    }

    /// <summary>Records that a given Doxygen html file exists in the source set.</summary>
    public void Register(string htmlFileName) => _known.Add(htmlFileName);

    /// <summary>
    /// Output markdown file path (relative to the wiki root) for a source html file.
    /// </summary>
    public string MarkdownFileFor(string htmlFileName) => WikiPath(htmlFileName) + ".md";

    /// <summary>
    /// Wiki path of a page relative to the wiki root, without extension.
    /// index.html -> "FPL-Documentation"; everything else -> "FPL-Documentation/&lt;base&gt;".
    /// </summary>
    private string WikiPath(string htmlFileName)
    {
        if (htmlFileName.Equals(IndexHtml, StringComparison.OrdinalIgnoreCase))
            return _indexSlug;
        string baseName = Path.GetFileNameWithoutExtension(htmlFileName);
        return $"{_folder}/{baseName}";
    }

    /// <summary>
    /// Rewrites a Doxygen href found on <paramref name="currentHtmlFile"/> into a
    /// wiki-compatible (relative) link target. Returns <c>null</c> to drop the link.
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

        // Pure in-page anchor: kept as-is; the converter emits matching
        // <a name="..."></a> markers so it resolves.
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

        // Internal Doxygen page -> relative wiki link.
        if (file.EndsWith(".html", StringComparison.OrdinalIgnoreCase))
        {
            // Link to the current page itself: prefer a plain in-page anchor.
            if (file.Equals(currentHtmlFile, StringComparison.OrdinalIgnoreCase))
                return anchor.Length > 0 ? $"#{anchor}" : WikiPath(file).Split('/')[^1];

            string rel = MakeRelative(WikiPath(currentHtmlFile), WikiPath(file));
            return anchor.Length > 0 ? $"{rel}#{anchor}" : rel;
        }

        // Anything else (images, css, ...) keeps its relative path.
        return href;
    }

    /// <summary>
    /// Relative path from one wiki file to another (both "/"-separated, no
    /// extension). The last segment of each path is the file name.
    /// </summary>
    private static string MakeRelative(string fromPath, string toPath)
    {
        string[] from = fromPath.Split('/');
        string[] to = toPath.Split('/');
        int fromDirLen = from.Length - 1;
        int toDirLen = to.Length - 1;

        int common = 0;
        while (common < fromDirLen && common < toDirLen &&
               string.Equals(from[common], to[common], StringComparison.OrdinalIgnoreCase))
            common++;

        var sb = new StringBuilder();
        for (int up = common; up < fromDirLen; up++)
            sb.Append("../");
        for (int down = common; down < toDirLen; down++)
            sb.Append(to[down]).Append('/');
        sb.Append(to[^1]);

        string rel = sb.ToString();
        return rel.Length == 0 ? to[^1] : rel;
    }
}
