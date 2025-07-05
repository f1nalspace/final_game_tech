using System.Collections.Generic;
using System.Linq;

namespace FPLApiGenerator
{
    class CValue
    {
        public CType Type { get; }
        public string Value { get; }

        public CValue(CType type, string value)
        {
            Type = type;
            Value = value;
        }

        public override string ToString() => $"{Type} -> {Value}";

        private static readonly CValue[] _defaultValues = new[] {
            new CValue(CType.CharType, "\\0"),
            new CValue(CType.IntType, "0"),
            new CValue(CType.SizeType, "0U"),
            new CValue(CType.BoolType, "false"),
            new CValue(CType.U8Type, "0U"),
            new CValue(CType.U16Type, "0U"),
            new CValue(CType.U32Type, "0U"),
            new CValue(CType.U64Type, "0U"),
            new CValue(CType.S8Type, "0"),
            new CValue(CType.S16Type, "0"),
            new CValue(CType.S32Type, "0"),
            new CValue(CType.S64Type, "0"),
            new CValue(CType.StringType, "fpl_null"),
            new CValue(CType.B32Type, "0"),
            new CValue(CType.UPtrType, "0"),
        };

        private static readonly Dictionary<CType, string> _defaultValueMap = _defaultValues.ToDictionary(v => v.Type, v => v.Value);
        public static string GetDefaultValue(CType type) => _defaultValueMap.GetValueOrDefault(type);
    }
}
