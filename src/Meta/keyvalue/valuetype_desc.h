#pragma once

#include "valuetype_types.h"

#include "Generic/bits/array.h"

#include "Generic/keyvalue.h"

#include "table_forward.h"

#include "Generic/execution/concepts.h"

namespace Engine {

DERIVE_FUNCTION(customScopePtr)

template <typename T>
auto resolveCustomScopePtr(T *t)
{
    if constexpr (has_function_customScopePtr_v<T>) {
        if (t) {
            return t->customScopePtr();
        } else {
            using Ptr = decltype(t->customScopePtr());
            if constexpr (std::same_as<Ptr, ScopePtr>) {
                return Ptr { nullptr, table<decayed_t<T>> };
            } else {
                return Ptr { nullptr };
            }
        }
    } else {
        return ScopePtr { t, table<decayed_t<T>> };
    }
}

template <typename T>
    requires(!std::is_pointer_v<T>)
auto resolveCustomScopePtr(T &t)
{
    return resolveCustomScopePtr(&t);
}

template <typename T, bool keepPtr = false>
auto resolveHelper()
{
    using Ptr = decltype(resolveCustomScopePtr(std::declval<T&>()));
    if constexpr (std::same_as<Ptr, ScopePtr>) {
        return type_holder<T>;
    } else {
        if constexpr (keepPtr) {
            return type_holder<Ptr>;
        } else {
            return type_holder<std::remove_pointer_t<Ptr>>;
        }
    }
}

template <typename T, bool keepPtr = false>
using resolveCustomScopePtr_t = typename decltype(resolveHelper<T, keepPtr>())::type;

template <typename T>
concept ValueTypePrimitive = ValueTypeList::contains<T>;

enum class ValueTypeEnum : unsigned char {
#define VALUETYPE_SEP ,
#define VALUETYPE_TYPE(Name, Storage, ...) Name##Value
#include "valuetypedefinclude.h"
    ,
    MAX_VALUETYPE_TYPE
};

struct META_EXPORT ValueTypeIndex {
    ValueTypeEnum mIndex;

    constexpr ValueTypeIndex(ValueTypeEnum index = ValueTypeEnum::NullValue)
        : mIndex(index)
    {
    }

    constexpr operator ValueTypeEnum() const
    {
        return mIndex;
    }

    std::string_view toString() const;
    std::string_view toTypeName() const;
};

enum class ExtendedValueTypeEnum : unsigned char {
    GenericType = static_cast<unsigned char>(ValueTypeEnum::MAX_VALUETYPE_TYPE),
    OptionalType,
    MAX_EXTENDEDVALUETYPE_TYPE
};

struct META_EXPORT ExtendedValueTypeIndex {
    static constexpr size_t bitCount = std::bit_width(static_cast<unsigned char>(ExtendedValueTypeEnum::MAX_EXTENDEDVALUETYPE_TYPE) - 1u);

    BitArray<(sizeof(uintptr_t) * 8) / bitCount, bitCount, ExtendedValueTypeEnum> mTypeList;

    constexpr ExtendedValueTypeIndex(ValueTypeIndex t)
    {
        mTypeList[0] = static_cast<ExtendedValueTypeEnum>(t.mIndex);
    }

    constexpr ExtendedValueTypeIndex(ExtendedValueTypeEnum t)
    {
        mTypeList[0] = t;
    }

    constexpr ExtendedValueTypeIndex(ExtendedValueTypeIndex t, const ExtendedValueTypeIndex &inner)
    {
        mTypeList[0] = t;
        size_t i = 0;
        int mark = 1;
        do {
            --mark;
            mTypeList[i + 1] = inner.mTypeList[i];
            ++i;
            if (mTypeList[i + 1] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualAssociativeRangeValue)) {
                assert(mark == 0);
                mark = 2;
            }
            if (mTypeList[i + 1] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualSequenceRangeValue)) {
                assert(mark == 0);
                mark = 1;
            }
        } while (mark > 0);
    }

    constexpr ExtendedValueTypeIndex(ExtendedValueTypeIndex t, const ExtendedValueTypeIndex &innerKey, const ExtendedValueTypeIndex &innerValue)
    {
        mTypeList[0] = t;
        mTypeList[1] = innerKey.mTypeList[0];
        size_t i = 0;
        int mark = 1;
        do {
            --mark;
            mTypeList[i + 2] = innerValue.mTypeList[i];
            ++i;
            if (mTypeList[i + 2] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualAssociativeRangeValue)) {
                assert(mark == 0);
                mark = 2;
            }
            if (mTypeList[i + 2] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualSequenceRangeValue)) {
                assert(mark == 0);
                mark = 1;
            }
        } while (mark > 0);
    }

    constexpr bool canAccept(ValueTypeIndex valueType)
    {
        return mTypeList[0] == ExtendedValueTypeEnum::GenericType || static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[0])) == valueType;
    }

    constexpr bool isRegular(size_t level = 0) const
    {
        return static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[level])) < ValueTypeEnum::MAX_VALUETYPE_TYPE;
    }

    constexpr operator ExtendedValueTypeEnum() const
    {
        return mTypeList[0];
    }

    constexpr operator ValueTypeIndex() const
    {
        assert(isRegular());
        return static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[0]));
    }

    constexpr std::strong_ordering operator<=>(const ExtendedValueTypeIndex &other) const = default;
    constexpr bool operator==(const ExtendedValueTypeIndex &other) const = default;

    constexpr bool operator==(const ValueTypeIndex &other) const
    {
        return static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[0])) == other;
    }

    constexpr bool operator!=(const ValueTypeIndex &other) const
    {
        return static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[0])) != other;
    }

    constexpr bool operator==(const ExtendedValueTypeEnum &other) const
    {
        return mTypeList[0] == other;
    }

    constexpr bool operator!=(const ExtendedValueTypeEnum &other) const
    {
        return mTypeList[0] != other;
    }

    std::string_view toString(size_t level = 0) const;
};

union ValueTypeSecondaryTypeInfo {

    constexpr ValueTypeSecondaryTypeInfo()
        : mDummy(nullptr)
    {
    }

    constexpr ValueTypeSecondaryTypeInfo(nullptr_t)
        : mDummy(nullptr)
    {
    }

    constexpr ValueTypeSecondaryTypeInfo(const MetaTable **metaTable)
        : mMetaTable(metaTable)
    {
    }

    constexpr ValueTypeSecondaryTypeInfo(const FunctionTable **functionTable)
        : mFunctionTable(functionTable)
    {
    }

    constexpr ValueTypeSecondaryTypeInfo(const EnumMetaTable *enumTable)
        : mEnumTable(enumTable)
    {
    }

    std::strong_ordering compare(ValueTypeIndex index, const ValueTypeSecondaryTypeInfo &other) const
    {
        switch (index) {
        case ValueTypeEnum::ScopeValue:
            return *mMetaTable <=> *other.mMetaTable;
        case ValueTypeEnum::ApiFunctionValue:
        case ValueTypeEnum::BoundApiFunctionValue:
            return *mFunctionTable <=> *other.mFunctionTable;
        case ValueTypeEnum::EnumValue:
            return mEnumTable <=> other.mEnumTable;
        default:
            return mDummy <=> other.mDummy;
        }
    }

    const void *mDummy;
    const MetaTable **mMetaTable;
    const FunctionTable **mFunctionTable;
    const EnumMetaTable *mEnumTable;
};

struct META_EXPORT ValueTypeDesc {
    ValueTypeIndex mType;
    ValueTypeSecondaryTypeInfo mSecondary;

    bool canAccept(const ValueTypeDesc &valueType);
    std::string toString() const;
    std::string toTypeName() const;

    std::strong_ordering operator<=>(const ValueTypeDesc &other) const
    {
        if (auto cmp = mType <=> other.mType; cmp != 0)
            return cmp;
        return mSecondary.compare(mType, other.mSecondary);
    }
    bool operator==(const ValueTypeDesc &other) const
    {
        return mType == other.mType && mSecondary.compare(mType, other.mSecondary) == 0;
    }
};

struct META_EXPORT ExtendedValueTypeDesc {
    ExtendedValueTypeIndex mType;
    ValueTypeSecondaryTypeInfo mSecondary;

    constexpr ExtendedValueTypeDesc(ExtendedValueTypeIndex type, ValueTypeSecondaryTypeInfo secondary = {})
        : mType(type)
        , mSecondary(secondary)
    {
    }

    constexpr ExtendedValueTypeDesc(ExtendedValueTypeIndex type, const ExtendedValueTypeDesc &innerDesc)
        : mType(type, innerDesc.mType)
        , mSecondary(innerDesc.mSecondary)
    {
    }

    constexpr ExtendedValueTypeDesc(ExtendedValueTypeIndex type, const ExtendedValueTypeDesc &innerKeyDesc, const ExtendedValueTypeDesc &innerValueDesc)
        : mType(type, innerKeyDesc.mType, innerValueDesc.mType)
        , mSecondary(innerValueDesc.mSecondary)
    {
        assert(!innerKeyDesc.mSecondary.mDummy);
    }

    constexpr ExtendedValueTypeDesc(const ValueTypeDesc &desc)
        : mType(desc.mType)
        , mSecondary(desc.mSecondary)
    {
    }

    constexpr bool canAccept(const ValueTypeDesc &valueType) const
    {
        if (mType == ExtendedValueTypeEnum::GenericType)
            return true;
        return static_cast<ValueTypeDesc>(*this).canAccept(valueType);
    }

    constexpr bool isCompatible(const ExtendedValueTypeDesc &valueType)
    {
        if (mType.isRegular())
            return valueType.canAccept(*this);
        if (valueType.mType.isRegular())
            return canAccept(valueType);
        return mType == valueType.mType;
    }

    constexpr operator ValueTypeDesc() const
    {
        assert(mType.isRegular());
        return { mType, mSecondary };
    }

    constexpr std::strong_ordering operator<=>(const ExtendedValueTypeDesc &other) const
    {
        if (auto cmp = mType <=> other.mType; cmp != 0)
            return cmp;
        return mSecondary.compare(mType, other.mSecondary);
    }
    constexpr bool operator==(const ExtendedValueTypeDesc &other) const
    {
        return mType == other.mType && mSecondary.compare(mType, other.mSecondary) == 0;
    }

    std::string toString(size_t level = 0) const;
};

template <typename T>
constexpr ValueTypeIndex toValueTypeIndex()
{
    static_assert(!std::is_rvalue_reference_v<T>);
    if constexpr (ValueTypePrimitive<T>) {
        return static_cast<ValueTypeEnum>(ValueTypeList::index<size_t, T>);
    } else if constexpr (InstanceOf<T, Flags>) {
        return ValueTypeEnum::FlagsValue;
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
            return ValueTypeEnum::KeyValueVirtualSequenceRangeValue;
        else
            return ValueTypeEnum::KeyValueVirtualAssociativeRangeValue;
    } else if constexpr (Execution::Sender<T>) {
        return ValueTypeEnum::SenderValue;
    } else if constexpr (Pointer<T>) {
        if constexpr (std::is_function_v<std::remove_pointer_t<T>>)
            return ValueTypeEnum::FunctionValue;
        else if constexpr (std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, FunctionTable>)
            return ValueTypeEnum::ApiFunctionValue;
        else
            return ValueTypeEnum::ScopeValue;
    } else {
        return ValueTypeEnum::OwnedScopeValue;
    }
}

template <typename T>
constexpr ExtendedValueTypeDesc toValueTypeDesc()
{
    static_assert(!requires { typename std::decay_t<T>::no_value_type; });

    static_assert(!std::is_rvalue_reference_v<T>);
    if constexpr (InstanceOf<T, std::optional>) {
        return { { ExtendedValueTypeEnum::OptionalType }, toValueTypeDesc<typename is_instance<T, std::optional>::argument_types::template unpack_unique<>>() };
    } else if constexpr (ValueTypePrimitive<T>) {
        return { toValueTypeIndex<T>() };
    } else if constexpr (std::same_as<T, ValueType>) {
        return { ExtendedValueTypeEnum::GenericType };
    } else if constexpr (InstanceOf<T, Flags>) {
        return { { ValueTypeEnum::FlagsValue }, &T::Representation::sTable };
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumType> || InstanceOf<std::decay_t<T>, BaseEnum>) {
        return { { ValueTypeEnum::EnumValue }, &T::Representation::sTable };
    } else if constexpr (Execution::Sender<T>) {
        return { { ValueTypeEnum::SenderValue }, nullptr };
    } else if constexpr (std::same_as<T, ScopePtr>) {
        return { { ValueTypeEnum::ScopeValue }, static_cast<const MetaTable **>(nullptr) };
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
            return { { ValueTypeEnum::KeyValueVirtualSequenceRangeValue }, toValueTypeDesc<std::ranges::range_value_t<T>>() };
        else
            return { { ValueTypeEnum::KeyValueVirtualAssociativeRangeValue }, toValueTypeDesc<KeyType_t<std::ranges::range_value_t<T>>>(), toValueTypeDesc<ValueType_t<std::ranges::range_value_t<T>>>() };
    } else if constexpr (InstanceOfA<T, TypedBoundApiFunction>) {
        return { { ValueTypeEnum::BoundApiFunctionValue }, is_instance_auto<T, TypedBoundApiFunction>::arguments::value };
    } else if constexpr (Pointer<T>) {
        if constexpr (Function<std::remove_pointer_t<T>>)
            // return { { ValueTypeEnum::ApiFunctionValue }, nullptr };
            throw 0;
        else if constexpr (std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, FunctionTable>)
            // return { { ValueTypeEnum::ApiFunctionValue }, nullptr };
            throw 0;
        else {
            return { { ValueTypeEnum::ScopeValue }, &table<std::remove_pointer_t<resolveCustomScopePtr_t<T>>> };
        }
    } else if constexpr (InstanceOf<std::decay_t<T>, std::unique_ptr>) {
        return { { ValueTypeEnum::ScopeValue }, &table<typename is_instance<std::decay_t<T>, std::unique_ptr>::argument_types::first> };
    } else {
        return { { ValueTypeEnum::OwnedScopeValue }, &table<resolveCustomScopePtr_t<T>> };
    }
}
}