using System;
using System.Xml.Linq;

namespace FPLApiGenerator
{
    [Flags]
    enum CTypeFlags
    {
        None = 0,
        Pointer = 1 << 0,
        Constant = 1 << 1,
        OpaqueStruct = 1 << 2,
    }

    class CType : IEquatable<CType>
    {
        public string Name { get; set; }
        public CTypeFlags Flags { get; }

        public CType(string name, CTypeFlags flags = CTypeFlags.None)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(name);
            Name = name;
            Flags = flags;
        }

        public static readonly CType VoidType = new CType("void");
        public static readonly CType CharType = new CType("char");
        public static readonly CType IntType = new CType("int");
        public static readonly CType SizeType = new CType("size_t");
        public static readonly CType BoolType = new CType("bool");
        public static readonly CType U8Type = new CType("uint8_t");
        public static readonly CType U16Type = new CType("uint16_t");
        public static readonly CType U32Type = new CType("uint32_t");
        public static readonly CType U64Type = new CType("uint64_t");
        public static readonly CType S8Type = new CType("int8_t");
        public static readonly CType S16Type = new CType("int16_t");
        public static readonly CType S32Type = new CType("int32_t");
        public static readonly CType S64Type = new CType("int64_t");
        public static readonly CType StringType = new CType("char", CTypeFlags.Pointer);
        public static readonly CType B32Type = new CType("fpl_b32");
        public static readonly CType UPtrType = new CType("uintptr_t");

        public static readonly CType[] DefaultTypes =
        {
            VoidType,
            CharType,
            IntType,
            SizeType,
            BoolType,
            U8Type,
            U16Type,
            U32Type,
            U64Type,
            S8Type,
            S16Type,
            S32Type,
            S64Type,
            StringType,
            B32Type,
            UPtrType,
        };

        public override string ToString()
        {
            if (Flags.HasFlag(CTypeFlags.OpaqueStruct))
                return $"struct {Name}";
            else 
                return Name;
        }

        private static CTypeFlags GetFlagCompare(CTypeFlags flags)
        {
            CTypeFlags result = CTypeFlags.None;
            if (flags.HasFlag(CTypeFlags.Pointer))
                result |= CTypeFlags.Pointer;
            return result;
        }

        public bool Equals(CType other)
        {
            if (other is null)
                return false;
            if (!string.Equals(Name, other.Name))
                return false;
            if (GetFlagCompare(Flags) != GetFlagCompare(other.Flags))
                return false;
            return true;
        }
        public override bool Equals(object obj) => Equals(obj as CFunction);
        public override int GetHashCode() => HashCode.Combine(Name, GetFlagCompare(Flags));
    }
}
