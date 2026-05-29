Create a C#/.NET 10 Console-Tool that runs on linux that does the following:
- Convert a full generated doxygen documentations into markdown files that is 100% compatible with github wiki.
- Any linking to other pages must still work
- Repository root of the wiki is `/home/final/_projects/fpl/wiki/`
- `index.html` maps to `FPL-Documentation.md`
- All other pages are referenced under `FPL-Documentation.md`
- Project is called "doxygen-markdown-converter"
- Two arguments passed to the console app, first is the source folder, second is the target folder - an optional third argument defines the name of the "index.html" to markdown-file-mapping, which defaults to "FPL-Documentation.md" for now.
- Path to docs are: /home/final/_projects/fpl/docs
