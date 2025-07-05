using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Text;

namespace FPLApiGenerator
{
    class CFunction : IEquatable<CFunction>
    {
        public string Name { get; }
        public ImmutableArray<CArgument> Arguments { get; }
        public CType ResultType { get; }

        public CFunction(CType resultType, string name, params CArgument[] arguments)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(name);
            Name = name;
            ResultType = resultType ?? CType.VoidType;
            Arguments = arguments?.ToImmutableArray() ?? ImmutableArray<CArgument>.Empty;
        }

        public override string ToString()
        {
            StringBuilder s = new StringBuilder();
            s.Append(ResultType);
            s.Append(' ');
            s.Append(Name);
            s.Append('(');
            if (Arguments.Length == 0)
                s.Append("void");
            else
                s.Append(string.Join(", ", Arguments));
            s.Append(')');
            s.Append(';');
            return s.ToString();
        }

        public bool Equals(CFunction other) => other is not null && string.Equals(Name, other.Name, StringComparison.OrdinalIgnoreCase);
        public override bool Equals(object obj) => Equals(obj as CFunction);
        public override int GetHashCode() => Name.GetHashCode();
    }
}
