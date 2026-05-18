#pragma once

#include "Generic/bits/array.h"
#include "Generic/execution/binding.h"
#include "Generic/execution/concepts.h"
#include "Generic/keyvalue.h"

#include "../meta_decay.h"
#include "table_forward.h"
#include "valuetype_types.h"

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
                return Ptr { nullptr, table<meta_decayed_t<T>> };
            } else {
                return Ptr { nullptr };
            }
        }
    } else {
        return ScopePtr { t, table<meta_decayed_t<T>> };
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
    using Ptr = decltype(resolveCustomScopePtr(std::declval<T &>()));
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

template <typename T>
concept ValueTypeStorage = ValueTypeStorageList::contains<T>;

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
    VariantType,
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

    constexpr ExtendedValueTypeIndex(ExtendedValueTypeIndex t, auto &&inner)
    {
        mTypeList[0] = t;

        size_t offset = 1;
        for (ExtendedValueTypeIndex index : inner) {
            size_t i = 0;
            int mark = 1;
            do {
                --mark;
                ExtendedValueTypeEnum type = index.mTypeList[i];
                mTypeList[i + offset] = type;
                if (type == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualAssociativeRangeValue) || type == ExtendedValueTypeEnum::VariantType) {
                    mark += 2;
                }
                if (type == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualSequenceRangeValue) || type == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::BindingValue)) {
                    mark += 1;
                }
                ++i;
            } while (mark > 0);
            offset += i;
        }
    }

    constexpr bool canAccept(ValueTypeIndex valueType)
    {
        return mTypeList[0] == ExtendedValueTypeEnum::GenericType || static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[0])) == valueType;
    }

    constexpr bool isRegular(size_t level = 0) const
    {
        return static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[level])) < ValueTypeEnum::MAX_VALUETYPE_TYPE;
    }

    constexpr std::pair<ExtendedValueTypeIndex, ExtendedValueTypeIndex> unwrapVariant()
    {
        assert(mTypeList[0] == ExtendedValueTypeEnum::VariantType);

        std::pair<ExtendedValueTypeIndex, ExtendedValueTypeIndex> result { {}, {} };
        int i = 1;

        for (int j = 0; j < 2; ++j) {
            ExtendedValueTypeIndex &current = (&result.first)[j];

            int mark = 1;
            int out = 0;
            do {
                --mark;
                current.mTypeList[out] = mTypeList[i];
                if (current.mTypeList[out] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualAssociativeRangeValue) || current.mTypeList[out] == ExtendedValueTypeEnum::VariantType) {
                    assert(mark == 0);
                    mark = 2;
                }
                if (current.mTypeList[out] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualSequenceRangeValue) || current.mTypeList[out] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::BindingValue)) {
                    assert(mark == 0);
                    mark = 1;
                }
                ++i;
                ++out;
            } while (mark > 0);
        }

        return result;
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

private:
    constexpr ExtendedValueTypeIndex() = default;
};

union ValueTypeSecondaryTypeInfo {

    constexpr ValueTypeSecondaryTypeInfo()
        : mDummy(nullptr)
    {
    }

    constexpr ValueTypeSecondaryTypeInfo(std::nullptr_t)
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

    constexpr ExtendedValueTypeDesc(ExtendedValueTypeIndex type, std::initializer_list<ExtendedValueTypeDesc> innerDesc)
        : mType(type, innerDesc | std::views::transform(&ExtendedValueTypeDesc::mType))
        , mSecondary(innerDesc.begin()->mSecondary)
    {
        // assert(!innerKeyDesc.mSecondary.mDummy || !innerValueDesc.mSecondary.mDummy || innerKeyDesc.mSecondary.mDummy == innerValueDesc.mSecondary.mDummy);
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

    constexpr std::pair<ExtendedValueTypeDesc, ExtendedValueTypeDesc> unwrapVariant()
    {
        std::pair<ExtendedValueTypeIndex, ExtendedValueTypeIndex> indices = mType.unwrapVariant();
        return { { indices.first, mSecondary }, { indices.second, mSecondary } };
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
    } else if constexpr (Execution::AnySender<T>) {
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

template <typename T, bool isReferenceWrapped = false>
constexpr ExtendedValueTypeDesc toValueTypeDesc()
{
    static_assert(!std::is_rvalue_reference_v<T>);
    static_assert(!requires { typename std::decay_t<T>::no_value_type; });

    if constexpr (tag_invocable<convert_ValueType_t<isReferenceWrapped>, T>) {
        return toValueTypeDesc<std::decay_t<std::invoke_result_t<convert_ValueType_t<isReferenceWrapped>, T>>, isReferenceWrapped>();
    } else if constexpr (InstanceOf<T, std::reference_wrapper>) {
        return toValueTypeDesc<typename T::type, true>();
    } else if constexpr (InstanceOf<T, std::variant>) {
        return { { ExtendedValueTypeEnum::VariantType }, { toValueTypeDesc<typename is_instance<T, std::variant>::argument_types::template select<0>>(), toValueTypeDesc<typename is_instance<T, std::variant>::argument_types::template select<1>>() } };
    } else if constexpr (ValueTypePrimitive<T>) {
        return { toValueTypeIndex<T>() };
    } else if constexpr (std::same_as<T, ValueType>) {
        return { ExtendedValueTypeEnum::GenericType };
    } else if constexpr (InstanceOf<T, Flags>) {
        return { { ValueTypeEnum::FlagsValue }, &T::Representation::sTable };
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumImpl>) {
        return { { ValueTypeEnum::EnumValue }, &T::Representation::sTable };
    } else if constexpr (Execution::AnyBinding<T>) {
        return { { ValueTypeEnum::BindingValue }, { toValueTypeDesc<std::decay_t<typename T::type>>() } };
    } else if constexpr (Execution::AnySender<T>) {
        return { { ValueTypeEnum::SenderValue }, nullptr };
    } else if constexpr (std::same_as<T, ScopePtr>) {
        return { { ValueTypeEnum::ScopeValue }, static_cast<const MetaTable **>(nullptr) };
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
            return { { ValueTypeEnum::KeyValueVirtualSequenceRangeValue }, { toValueTypeDesc<std::ranges::range_value_t<T>, true>() } };
        else
            return { { ValueTypeEnum::KeyValueVirtualAssociativeRangeValue }, { toValueTypeDesc<KeyType_t<std::ranges::range_reference_t<T>>, true>(), toValueTypeDesc<ValueType_t<std::ranges::range_reference_t<T>>, true>() } };
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
        return { { ValueTypeEnum::OwnedScopeValue }, &table<meta_decayed_t<resolveCustomScopePtr_t<T>>> };
    }
}


template <bool isReferenceWrapped>
struct convert_ValueType_t {

    template <typename T>
        requires(!tag_invocable<convert_ValueType_t, T>)
    decltype(auto) operator()(T &&t) const
    {
        static_assert(!requires { typename std::decay_t<T>::no_value_type; });

        if constexpr (InstanceOf<std::decay_t<T>, std::reference_wrapper>) {
            return convert_ValueType_t<true> {}(t.get());
        } else if constexpr (ValueTypePrimitive<std::decay_t<T>> || std::same_as<ValueType, std::decay_t<T>>) {
            return std::forward<T>(t);
        } else if constexpr (String<std::decay_t<T>>) {
            return std::string { std::forward<T>(t) };
        } else if constexpr (std::ranges::range<T>) {
            if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
                return KeyValueVirtualSequenceRange { std::forward<T>(t), type_holder<Functor_to_ValueType> };
            else
                return KeyValueVirtualAssociativeRange { std::forward<T>(t) };
        } else if constexpr (std::is_enum_v<std::decay_t<T>>) {
            if constexpr (std::is_reference_v<T>) {
                return static_cast<std::underlying_type_t<T> &>(t);
            } else {
                return static_cast<std::underlying_type_t<T>>(t);
            }
        } else if constexpr (InstanceOf<std::decay_t<T>, EnumImpl>) {
            return EnumHolder { std::forward<T>(t) };
        } else if constexpr (InstanceOf<std::decay_t<T>, Flags>) {
            return FlagsHolder { std::forward<T>(t) };
        } else if constexpr (Execution::AnyBinding<std::decay_t<T>>) {
            using Inner = decltype(convert_ValueType_t<false> {}(std::declval<forward_ref_t<typename std::decay_t<T>::type>>()));
            if constexpr (OneOf<Inner, ScopePtr, OwnedScopePtr>) {
                return KeyValueScopeBinding { std::forward<T>(t), table<std::remove_pointer_t<std::decay_t<typename std::decay_t<T>::type>>> };
            } else {
                return KeyValueBinding { std::forward<T>(t), toValueTypeIndex<std::decay_t<typename std::decay_t<T>::type>>() };
            }
        } else if constexpr (Execution::AnySender<std::decay_t<T>>) {
            return KeyValueSender { std::forward<T>(t) };
        } else if constexpr (InstanceOfA<std::decay_t<T>, TypedBoundApiFunction>) {
            return BoundApiFunction { std::forward<T>(t) };
        } else if constexpr (Pointer<std::decay_t<T>>) {
            return ScopePtr { t };
        } else if constexpr (InstanceOf<std::decay_t<T>, std::unique_ptr>) {
            return ScopePtr { t.get() };
        } else if constexpr (isReferenceWrapped) {
            return ScopePtr { &t };
        } else {
            return OwnedScopePtr { std::forward<T>(t) };
        }
        // static_assert(dependent_bool<T, false>::value, "The provided type can not be converted to a ValueType");
    }

    template <typename T>
    friend std::variant<T, std::monostate> tag_invoke(convert_ValueType_t, std::optional<T> &&o)
    {
        if (o) {
            return { std::move(*o) };
        } else {
            return { std::monostate {} };
        }
    }

    template <typename T>
    friend std::variant<std::reference_wrapper<T>, std::monostate> tag_invoke(convert_ValueType_t, std::optional<T> &o)
    {
        if (o) {
            return { *o };
        } else {
            return { std::monostate {} };
        }
    }

    template <typename T>
        requires tag_invocable<convert_ValueType_t, T>
    decltype(auto) operator()(T &&t) const
    {
        return tag_invoke(*this, std::forward<T>(t));
    }
};

inline constexpr convert_ValueType_t<false> convert_ValueType {};


}