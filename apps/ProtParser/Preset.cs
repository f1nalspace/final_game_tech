using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ProtParser
{
    class Preset
    {
        private readonly Dictionary<string, string> _properties = new Dictionary<string, string>();

        private readonly Dictionary<string, string> _procedures = new Dictionary<string, string>();

        private readonly List<string> _sources = new List<string>();
        public IEnumerable<string> Sources
        {
            get { return _sources; }
        }

        public IEnumerable<string> PropertyNames
        {
            get { return _properties.Keys; }
        }

        public string GetProperty(string name, string def = "")
        {
            if (!_properties.TryGetValue(name, out string value))
                return def;
            return value;
        }

        public void SetProperty(string name, string value)
        {
            if (!_properties.ContainsKey(name))
                _properties.Add(name, value);
            else
                _properties[name] = value;
        }

        public string GetProcedure(string name, string def = "")
        {
            if (!_procedures.TryGetValue(name, out string value))
                return def;
            return value;
        }

        public void SetProcedure(string name, string value)
        {
            if (!_procedures.ContainsKey(name))
                _procedures.Add(name, value);
            else
                _procedures[name] = value;
        }

        public void AddSource(string source)
        {
            _sources.Add(source);
        }

        enum SectionType
        {
            None,
            Settings,
            Sources,
            Procedures,
        }

        public static Preset Load(string filename)
        {
            Preset result = new Preset();
            using (StreamReader reader = new StreamReader(filename, Encoding.UTF8))
            {
                SectionType section = SectionType.None;
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    if (line.Length > 2 && line.StartsWith("[") && line.EndsWith("]"))
                    {
                        string sectionName = line.Substring(1, line.Length - 2);
                        switch (sectionName.ToLower())
                        {
                            case "settings":
                                section = SectionType.Settings;
                                break;
                            case "procedures":
                                section = SectionType.Procedures;
                                break;
                            case "sources":
                                section = SectionType.Sources;
                                break;
                            default:
                                section = SectionType.None;
                                break;
                        }
                    }
                    else if (line.Length > 0)
                    {
                        switch (section)
                        {
                            case SectionType.Settings:
                                {
                                    int equalPos = line.IndexOf("=");
                                    if (equalPos > -1)
                                    {
                                        string name = line.Substring(0, equalPos);
                                        string value = line.Substring(equalPos + 1);
                                        result.SetProperty(name, value);
                                    }
                                }
                                break;

                            case SectionType.Procedures:
                            {
                                int equalPos = line.IndexOf("=");
                                if (equalPos > -1)
                                {
                                    string name = line.Substring(0, equalPos);
                                    string value = line.Substring(equalPos + 1);
                                    result.SetProcedure(name, value);
                                }
                            }
                            break;

                            case SectionType.Sources:
                                {
                                    result.AddSource(line);
                                }
                                break;
                        }
                    }

                }
            }
            return (result);
        }

        public void Save(string filename)
        {
            using (StreamWriter writer = new StreamWriter(filename, false, Encoding.UTF8))
            {
                writer.WriteLine("[Settings]");
                foreach (var property in _properties.OrderBy(p => p.Key))
                    writer.WriteLine($"{property.Key}={property.Value}");

                writer.WriteLine();

                writer.WriteLine("[Procedures]");
                foreach (var procedure in _procedures.OrderBy(p => p.Key))
                    writer.WriteLine($"{procedure.Key}={procedure.Value}");

                writer.WriteLine();
 
                writer.WriteLine("[Sources]");
                foreach (var source in _sources)
                    writer.WriteLine(source);
            }
        }
    }
}
