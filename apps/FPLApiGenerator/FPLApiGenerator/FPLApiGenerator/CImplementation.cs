namespace FPLApiGenerator
{
    class CImplementation
    {
        public string Name { get; }
        public EightCC Code { get; }

        public CImplementation(string name, EightCC code)
        {
            Name = name;
            Code = code;
        }

        public override string ToString() => $"{Name} [{Code}]";
    }
}
