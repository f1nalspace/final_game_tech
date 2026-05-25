#pragma once

#include <final_ftd.h>

// C++ helpers for defining FTD schema tables.
//
// Eliminates the repetition (and the foot-guns) of writing ftdField / ftdType
// literals by hand:
//   - field name is derived from the C++ member identifier (so it cannot
//     drift from the actual member),
//   - offsetof is filled in automatically against the correct struct type,
//   - scalar field kinds are deduced from the member's declared type via
//     ftd::KindOf,
//   - sub-struct fields auto-resolve their ftdType pointer via ftd::TypeOf
//     once the type has been bound with FTD_BIND_TYPE,
//   - type tables stringize the C++ type name and pull size/align from the
//     compiler instead of being typed twice.

namespace ftd {
    // Scalar/string kind deduced from member type. Defaults to Struct so that
    // nested aggregates fall through to FTD_FIELD_STRUCT semantics.
    template<typename T>
    struct KindOf {
        static constexpr ftdFieldKind value = ftdFieldKind_Struct;
    };

#define FTD__SCALAR_KIND(CType, K) \
	template<> struct KindOf<CType> { static constexpr ftdFieldKind value = K; }

    FTD__SCALAR_KIND(bool, ftdFieldKind_Bool);
    FTD__SCALAR_KIND(int8_t, ftdFieldKind_S8);
    FTD__SCALAR_KIND(int16_t, ftdFieldKind_S16);
    FTD__SCALAR_KIND(int32_t, ftdFieldKind_S32);
    FTD__SCALAR_KIND(int64_t, ftdFieldKind_S64);
    FTD__SCALAR_KIND(uint8_t, ftdFieldKind_U8);
    FTD__SCALAR_KIND(uint16_t, ftdFieldKind_U16);
    FTD__SCALAR_KIND(uint32_t, ftdFieldKind_U32);
    FTD__SCALAR_KIND(uint64_t, ftdFieldKind_U64);
    FTD__SCALAR_KIND(float, ftdFieldKind_F32);
    FTD__SCALAR_KIND(double, ftdFieldKind_F64);
    FTD__SCALAR_KIND(const char *, ftdFieldKind_String);
    FTD__SCALAR_KIND(char *, ftdFieldKind_String);

#undef FTD__SCALAR_KIND

    // Look up table from a C++ type to its registered ftdType. Must be bound
    // per-type with FTD_BIND_TYPE; the unspecialized primary fires a
    // static_assert so a missing binding is a compile error, not a silent
    // nullptr subtype at runtime.
    template<typename T>
    struct TypeOf {
        static_assert(sizeof(T) == 0,
            "ftd::TypeOf<T> is not bound. Call FTD_BIND_TYPE(T, <ftdType>) at "
            "namespace scope after the ftdType is defined, before using "
            "FTD_FIELD_STRUCT for a member of type T.");
        static constexpr const ftdType *value = nullptr;
    };
} // namespace ftd

// Bind a C++ type to its ftdType so FTD_FIELD_STRUCT can auto-resolve subtype.
// Must be invoked at namespace scope, after the ftdType is defined.
#define FTD_BIND_TYPE(CType, ftdTypeRef) \
	namespace ftd { template<> struct TypeOf<CType> { \
		static constexpr const ftdType *value = &(ftdTypeRef); \
	}; }

// Scalar / string field. Kind is deduced from decltype(StructT::member).
#define FTD_FIELD(StructT, member) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), \
	          ::ftd::KindOf<decltype(StructT::member)>::value }

// Nested-struct field; subtype is looked up via ftd::TypeOf<member type>.
#define FTD_FIELD_STRUCT(StructT, member) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), \
	          ftdFieldKind_Struct, \
	          ::ftd::TypeOf<decltype(StructT::member)>::value }

// Enum field; pass the ftdType pointer of the enum.
#define FTD_FIELD_ENUM(StructT, member, enumTypePtr) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), \
	          ftdFieldKind_Enum, (enumTypePtr) }

// Reference field (pointer-to-T). Pass the ftdType pointer of T.
#define FTD_FIELD_REF(StructT, member, refTypePtr) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), \
	          ftdFieldKind_Ref, (refTypePtr) }

// Union arm. The discriminator string names a sibling enum field; unionTag is
// the enum value that selects this arm.
#define FTD_FIELD_UNION(StructT, member, subTypePtr, discField, unionTag) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), \
	          ftdFieldKind_Union, (subTypePtr), (discField), \
	          (uint32_t)(unionTag) }

// Field with an explicitly specified kind (e.g. MemoryData8 / MemorySize /
// ArrayCount). Subtype defaults to nullptr; pass another arg-form if needed.
#define FTD_FIELD_KIND(StructT, member, kind) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), (kind) }

#define FTD_FIELD_KIND_SUB(StructT, member, kind, subTypePtr) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), (kind), (subTypePtr) }

#define FTD_FIELD_ARRAY_DATA(StructT, member) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), ftdFieldKind_ArrayData, nullptr }

#define FTD_FIELD_ARRAY_SIZE(StructT, member) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), ftdFieldKind_ArrayCount, nullptr }

#define FTD_FIELD_ARRAY_CAPACITY(StructT, member) \
	ftdField{ #member, (uint32_t)offsetof(StructT, member), ftdFieldKind_ArrayCapacity, nullptr }

#define FTD_FIELD_HIDDEN(name) \
	ftdField{ name, 0, ftdFieldKind_String, nullptr, 0, ftdFieldFlag_Hidden }

// Struct ftdType literal. Name is stringized from the C++ type.
#define FTD_TYPE_STRUCT(StructT, FieldsArr) \
	ftdType{ .name = #StructT, .size = sizeof(StructT), .align = alignof(StructT), \
	         .fields = (FieldsArr), .fieldCount = (uint32_t)fplArrayCount(FieldsArr) }

// Enum ftdType literal. The fields/fieldCount entries are left default-zero.
#define FTD_TYPE_ENUM(EnumT, ValuesArr) \
	ftdType{ .name = #EnumT, .size = sizeof(EnumT), .align = alignof(EnumT), \
	         .enumValues = (ValuesArr), \
	         .enumValueCount = (uint32_t)fplArrayCount(ValuesArr) }

// Enum value literal for a scoped enum (enum class EnumT { Foo, ... }).
#define FTD_ENUM_VALUE(EnumT, valueName) \
	ftdEnumValue{ #valueName, (int32_t)EnumT::valueName }
