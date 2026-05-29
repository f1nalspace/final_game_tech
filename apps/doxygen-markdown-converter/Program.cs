using System.Text;

namespace doxygen_markdown_converter;

/// <summary>
/// Console entry point. Converts a full Doxygen HTML documentation tree into
/// flat Markdown files that are compatible with a GitHub wiki.
///
/// Usage:
///   doxygen-markdown-converter &lt;source-dir&gt; &lt;target-dir&gt; [index-map-name]
///
///   source-dir       Folder containing the generated Doxygen *.html files.
///   target-dir       Folder (wiki repository root) where *.md files are written.
///   index-map-name   Optional. Markdown file name that "index.html" maps to.
///                    Defaults to "Final-Platform-Layer.md".
/// </summary>
internal static class Program
{
    private const string DefaultIndexMapName = "Final-Platform-Layer.md";

    private static int Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;

        if (args.Length is < 2 or > 3)
        {
            Console.Error.WriteLine(
                "Usage: doxygen-markdown-converter <source-dir> <target-dir> [index-map-name]");
            Console.Error.WriteLine(
                $"  index-map-name defaults to \"{DefaultIndexMapName}\".");
            return 1;
        }

        string sourceDir = Path.GetFullPath(args[0]);
        string targetDir = Path.GetFullPath(args[1]);
        string indexMapName = args.Length == 3 && !string.IsNullOrWhiteSpace(args[2])
            ? args[2]
            : DefaultIndexMapName;

        if (!indexMapName.EndsWith(".md", StringComparison.OrdinalIgnoreCase))
            indexMapName += ".md";

        if (!Directory.Exists(sourceDir))
        {
            Console.Error.WriteLine($"Source folder does not exist: {sourceDir}");
            return 1;
        }

        Directory.CreateDirectory(targetDir);

        string[] htmlFiles = Directory
            .GetFiles(sourceDir, "*.html", SearchOption.TopDirectoryOnly)
            .Where(f => !IsIgnored(Path.GetFileName(f)))
            .OrderBy(f => f, StringComparer.Ordinal)
            .ToArray();

        if (htmlFiles.Length == 0)
        {
            Console.Error.WriteLine($"No *.html files found in: {sourceDir}");
            return 1;
        }

        // Pass 1: read every page title so file names (and therefore the wiki's
        // page titles) and inter-page links can be derived up front.
        var titles = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        foreach (string file in htmlFiles)
        {
            string name = Path.GetFileName(file);
            string? title = null;
            try { title = DoxygenMarkdownConverter.ReadTitle(file); }
            catch { /* fall back to the file name below */ }
            titles[name] = string.IsNullOrWhiteSpace(title)
                ? Path.GetFileNameWithoutExtension(name)
                : title!;
        }

        var linkMap = new LinkMap(indexMapName, titles);
        var converter = new DoxygenMarkdownConverter(linkMap);

        int converted = 0;
        int failed = 0;
        foreach (string file in htmlFiles)
        {
            string htmlName = Path.GetFileName(file);
            string mdName = linkMap.MarkdownFileFor(htmlName);
            string outPath = Path.Combine(targetDir, mdName.Replace('/', Path.DirectorySeparatorChar));
            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(outPath)!);
                string markdown = converter.ConvertFile(file);
                File.WriteAllText(outPath, markdown, new UTF8Encoding(false));
                converted++;
                Console.WriteLine($"  {htmlName,-48} -> {mdName}");
            }
            catch (Exception ex)
            {
                failed++;
                Console.Error.WriteLine($"  FAILED {htmlName}: {ex.Message}");
            }
        }

        // Generate the custom navigation sidebar (mirrors Doxygen's structure).
        try
        {
            string indexTitle = titles.TryGetValue("index.html", out string? it) && !string.IsNullOrWhiteSpace(it)
                ? it
                : Path.GetFileNameWithoutExtension(indexMapName);
            string sidebar = new SidebarBuilder(sourceDir, linkMap).Build(indexTitle);
            File.WriteAllText(Path.Combine(targetDir, "_Sidebar.md"), sidebar, new UTF8Encoding(false));
            Console.WriteLine("  _Sidebar.md (navigation) written");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"  WARN: could not write _Sidebar.md: {ex.Message}");
        }

        Console.WriteLine();
        Console.WriteLine($"Done. {converted} converted, {failed} failed -> {targetDir}");
        return failed == 0 ? 0 : 2;
    }

    /// <summary>Doxygen helper pages that carry no real documentation content.</summary>
    private static bool IsIgnored(string fileName) => fileName switch
    {
        "doxygen_crawl.html" => true, // search-engine crawl helper, links only
        _ => false,
    };
}
