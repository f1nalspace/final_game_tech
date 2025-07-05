using System;
using System.Text;

namespace FPLApiGenerator
{
    [Flags]
    enum CArgumentFlags
    {
        None = 0,
        Builtin = 1 << 0,
    }

    class CArgument : IEquatable<CArgument>
    {
        public string Name { get; }
        public CType Type { get; }
        public CArgumentFlags Flags { get; }

        public CArgument(CType type, string name, CArgumentFlags flags = CArgumentFlags.None)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(name);
            Name = name;
            Type = type;
            Flags = flags;
        }

        public override string ToString()
        {
            string constKeyword = Type.Flags.HasFlag(CTypeFlags.Constant) ? "const " : string.Empty;
            if (Type.Flags.HasFlag(CTypeFlags.Pointer))
                return $"{constKeyword}{Type} *{Name}";
            else
                return $"{constKeyword}{Type} {Name}";
        }

        public bool Equals(CArgument other) => other is not null && string.Equals(Name, other.Name, StringComparison.OrdinalIgnoreCase);
        public override bool Equals(object obj) => Equals(obj as CFunction);
        public override int GetHashCode() => Name.GetHashCode();
    }
}
