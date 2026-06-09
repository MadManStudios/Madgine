#pragma once

#include "Generic/containers/bits/array.h"
#include "Generic/execution/concepts.h"
#include "Generic/keyvalue.h"

#include "../meta_decay.h"
#include "table_forward.h"

namespace Engine {
namespace Reflect {

    using TypeList = type_pack<
#define VALUE_SEP ,
#define VALUE_TYPE(Name, Storage, ...) type_pack<__VA_ARGS__>::transform<std::decay_t>
#include "valuedefinclude.h"
        >;

    using ValueStorageList = type_pack<
#define VALUE_SEP ,
#define VALUE_TYPE(Name, Storage, ...) std::decay_t<Storage>
#include "valuedefinclude.h"
        >;

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
    concept PrimitiveType = TypeList::contains<T>;

    template <typename T>
    concept ValueStorage = ValueStorageList::contains<T>;

    enum class TypeEnum : unsigned char {
#define VALUE_SEP ,
#define VALUE_TYPE(Name, Storage, ...) Name##Value
#include "valuedefinclude.h"
        ,
        MAX
    };

    struct META_EXPORT TypeIndex {
        TypeEnum mIndex;

        constexpr TypeIndex(TypeEnum index = TypeEnum::NullValue)
            : mIndex(index)
        {
        }

        constexpr operator TypeEnum() const
        {
            return mIndex;
        }

        std::string_view toString() const;
        std::string_view toTypeName() const;
    };

    enum class ExtendedTypeEnum : unsigned char {
        GenericType = static_cast<unsigned char>(TypeEnum::MAX),
        VariantType,
        EXTENDED_MAX
    };

    struct META_EXPORT ExtendedTypeIndex {
        static constexpr size_t bitCount = std::bit_width(static_cast<unsigned char>(ExtendedTypeEnum::EXTENDED_MAX) - 1u);

        Containers::BitArray<(sizeof(uintptr_t) * 8) / bitCount, bitCount, ExtendedTypeEnum> mTypeList;

        constexpr ExtendedTypeIndex(TypeIndex t)
        {
            mTypeList[0] = static_cast<ExtendedTypeEnum>(t.mIndex);
        }

        constexpr ExtendedTypeIndex(ExtendedTypeEnum t)
        {
            mTypeList[0] = t;
        }

        constexpr ExtendedTypeIndex(ExtendedTypeIndex t, auto &&inner)
        {
            mTypeList[0] = t;

            size_t offset = 1;
            for (ExtendedTypeIndex index : inner) {
                size_t i = 0;
                int mark = 1;
                do {
                    --mark;
                    ExtendedTypeEnum type = index.mTypeList[i];
                    mTypeList[i + offset] = type;
                    if (type == static_cast<ExtendedTypeEnum>(TypeEnum::AssociativeRangeValue) || type == ExtendedTypeEnum::VariantType) {
                        mark += 2;
                    }
                    if (type == static_cast<ExtendedTypeEnum>(TypeEnum::SequenceRangeValue) || type == static_cast<ExtendedTypeEnum>(TypeEnum::BindingValue)) {
                        mark += 1;
                    }
                    ++i;
                } while (mark > 0);
                offset += i;
            }
        }

        constexpr bool canAccept(TypeIndex valueType)
        {
            return mTypeList[0] == ExtendedTypeEnum::GenericType || static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[0])) == valueType;
        }

        constexpr bool isRegular(size_t level = 0) const
        {
            return static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[level])) < TypeEnum::MAX;
        }

        constexpr std::pair<ExtendedTypeIndex, ExtendedTypeIndex> unwrapVariant()
        {
            assert(mTypeList[0] == ExtendedTypeEnum::VariantType);

            std::pair<ExtendedTypeIndex, ExtendedTypeIndex> result { {}, {} };
            int i = 1;

            for (int j = 0; j < 2; ++j) {
                ExtendedTypeIndex &current = (&result.first)[j];

                int mark = 1;
                int out = 0;
                do {
                    --mark;
                    current.mTypeList[out] = mTypeList[i];
                    if (current.mTypeList[out] == static_cast<ExtendedTypeEnum>(TypeEnum::AssociativeRangeValue) || current.mTypeList[out] == ExtendedTypeEnum::VariantType) {
                        assert(mark == 0);
                        mark = 2;
                    }
                    if (current.mTypeList[out] == static_cast<ExtendedTypeEnum>(TypeEnum::SequenceRangeValue) || current.mTypeList[out] == static_cast<ExtendedTypeEnum>(TypeEnum::BindingValue)) {
                        assert(mark == 0);
                        mark = 1;
                    }
                    ++i;
                    ++out;
                } while (mark > 0);
            }

            return result;
        }

        constexpr operator ExtendedTypeEnum() const
        {
            return mTypeList[0];
        }

        constexpr operator TypeIndex() const
        {
            assert(isRegular());
            return static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[0]));
        }

        constexpr std::strong_ordering operator<=>(const ExtendedTypeIndex &other) const = default;
        constexpr bool operator==(const ExtendedTypeIndex &other) const = default;

        constexpr bool operator==(const TypeIndex &other) const
        {
            return static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[0])) == other;
        }

        constexpr bool operator!=(const TypeIndex &other) const
        {
            return static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[0])) != other;
        }

        constexpr bool operator==(const ExtendedTypeEnum &other) const
        {
            return mTypeList[0] == other;
        }

        constexpr bool operator!=(const ExtendedTypeEnum &other) const
        {
            return mTypeList[0] != other;
        }

        std::string_view toString(size_t level = 0) const;

    private:
        constexpr ExtendedTypeIndex() = default;
    };

    union SecondaryTypeInfo {

        constexpr SecondaryTypeInfo()
            : mDummy(nullptr)
        {
        }

        constexpr SecondaryTypeInfo(std::nullptr_t)
            : mDummy(nullptr)
        {
        }

        constexpr SecondaryTypeInfo(const MetaTable **metaTable)
            : mMetaTable(metaTable)
        {
        }

        constexpr SecondaryTypeInfo(const FunctionTable **functionTable)
            : mFunctionTable(functionTable)
        {
        }

        constexpr SecondaryTypeInfo(const EnumMetaTable *enumTable)
            : mEnumTable(enumTable)
        {
        }

        std::strong_ordering compare(TypeIndex index, const SecondaryTypeInfo &other) const
        {
            switch (index) {
            case TypeEnum::ScopeValue:
                return *mMetaTable <=> *other.mMetaTable;
            case TypeEnum::ApiFunctionValue:
            case TypeEnum::BoundApiFunctionValue:
                return *mFunctionTable <=> *other.mFunctionTable;
            case TypeEnum::EnumValue:
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

    struct META_EXPORT Type {
        TypeIndex mType;
        SecondaryTypeInfo mSecondary;

        bool canAccept(const Type &valueType);
        std::string toString() const;
        std::string toTypeName() const;

        std::strong_ordering operator<=>(const Type &other) const
        {
            if (auto cmp = mType <=> other.mType; cmp != 0)
                return cmp;
            return mSecondary.compare(mType, other.mSecondary);
        }
        bool operator==(const Type &other) const
        {
            return mType == other.mType && mSecondary.compare(mType, other.mSecondary) == 0;
        }
    };

    struct META_EXPORT ExtendedType {
        ExtendedTypeIndex mType;
        SecondaryTypeInfo mSecondary;

        constexpr ExtendedType(ExtendedTypeIndex type, SecondaryTypeInfo secondary = {})
            : mType(type)
            , mSecondary(secondary)
        {
        }

        constexpr ExtendedType(ExtendedTypeIndex type, std::initializer_list<ExtendedType> innerDesc)
            : mType(type, innerDesc | std::views::transform(&ExtendedType::mType))
            , mSecondary(innerDesc.begin()->mSecondary)
        {
            // assert(!innerKeyDesc.mSecondary.mDummy || !innerValueDesc.mSecondary.mDummy || innerKeyDesc.mSecondary.mDummy == innerValueDesc.mSecondary.mDummy);
        }

        constexpr ExtendedType(const Type &desc)
            : mType(desc.mType)
            , mSecondary(desc.mSecondary)
        {
        }

        constexpr bool canAccept(const Type &valueType) const
        {
            if (mType == ExtendedTypeEnum::GenericType)
                return true;
            return static_cast<Type>(*this).canAccept(valueType);
        }

        constexpr bool isCompatible(const ExtendedType &valueType)
        {
            if (mType.isRegular())
                return valueType.canAccept(*this);
            if (valueType.mType.isRegular())
                return canAccept(valueType);
            return mType == valueType.mType;
        }

        constexpr std::pair<ExtendedType, ExtendedType> unwrapVariant()
        {
            std::pair<ExtendedTypeIndex, ExtendedTypeIndex> indices = mType.unwrapVariant();
            return { { indices.first, mSecondary }, { indices.second, mSecondary } };
        }

        constexpr operator Type() const
        {
            assert(mType.isRegular());
            return { mType, mSecondary };
        }

        constexpr std::strong_ordering operator<=>(const ExtendedType &other) const
        {
            if (auto cmp = mType <=> other.mType; cmp != 0)
                return cmp;
            return mSecondary.compare(mType, other.mSecondary);
        }
        constexpr bool operator==(const ExtendedType &other) const
        {
            return mType == other.mType && mSecondary.compare(mType, other.mSecondary) == 0;
        }

        std::string toString(size_t level = 0) const;
    };

    template <typename T>
    constexpr TypeIndex toTypeIndex()
    {
        static_assert(!std::is_rvalue_reference_v<T>);
        if constexpr (PrimitiveType<T>) {
            return static_cast<TypeEnum>(TypeList::index<size_t, T>);
        } else if constexpr (Concepts::InstanceOf<T, Engine::Flags>) {
            return TypeEnum::FlagsValue;
        } else if constexpr (std::ranges::range<T>) {
            if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
                return TypeEnum::SequenceRangeValue;
            else
                return TypeEnum::AssociativeRangeValue;
        } else if constexpr (Execution::AnySender<T>) {
            return TypeEnum::SenderValue;
        } else if constexpr (Concepts::Pointer<T>) {
            if constexpr (std::is_function_v<std::remove_pointer_t<T>>)
                return TypeEnum::FunctionValue;
            else if constexpr (std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, FunctionTable>)
                return TypeEnum::ApiFunctionValue;
            else
                return TypeEnum::ScopeValue;
        } else {
            return TypeEnum::OwnedScopeValue;
        }
    }

    template <typename T, bool isReferenceWrapped = false>
    constexpr ExtendedType toType()
    {
        static_assert(!std::is_rvalue_reference_v<T>);
        static_assert(!requires { typename std::decay_t<T>::no_value_type; });

        if constexpr (tag_invocable<convert_Value_t<isReferenceWrapped>, T>) {
            return toType<std::decay_t<std::invoke_result_t<convert_Value_t<isReferenceWrapped>, T>>, isReferenceWrapped>();
        } else if constexpr (Concepts::InstanceOf<T, std::reference_wrapper>) {
            return toType<typename T::type, true>();
        } else if constexpr (Concepts::InstanceOf<T, std::variant>) {
            return { { ExtendedTypeEnum::VariantType }, { toType<typename Concepts::is_instance<T, std::variant>::argument_types::template select<0>>(), toType<typename Concepts::is_instance<T, std::variant>::argument_types::template select<1>>() } };
        } else if constexpr (PrimitiveType<T>) {
            return { toTypeIndex<T>() };
        } else if constexpr (std::same_as<T, Value>) {
            return { ExtendedTypeEnum::GenericType };
        } else if constexpr (Concepts::InstanceOf<T, Engine::Flags>) {
            return { { TypeEnum::FlagsValue }, &T::Representation::sTable };
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, EnumImpl>) {
            return { { TypeEnum::EnumValue }, &T::Representation::sTable };
        } else if constexpr (Execution::AnyBinding<T>) {
            constexpr ExtendedType inner = toType<std::decay_t<typename std::decay_t<T>::type>, true>();
            if constexpr (inner.mType == TypeEnum::ScopeValue || inner.mType == TypeEnum::OwnedScopeValue) {
                return { { TypeEnum::ScopeBindingValue }, &table<std::remove_pointer_t<std::decay_t<typename std::decay_t<T>::type>>> };
            } else {
                return { { TypeEnum::BindingValue }, { inner } };
            }
        } else if constexpr (Execution::AnySender<T>) {
            return { { TypeEnum::SenderValue }, nullptr };
        } else if constexpr (std::same_as<T, ScopePtr>) {
            return { { TypeEnum::ScopeValue }, static_cast<const MetaTable **>(nullptr) };
        } else if constexpr (std::ranges::range<T>) {
            if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
                return { { TypeEnum::SequenceRangeValue }, { toType<std::ranges::range_value_t<T>, true>() } };
            else
                return { { TypeEnum::AssociativeRangeValue }, { toType<KeyType_t<std::ranges::range_reference_t<T>>, true>(), toType<ValueType_t<std::ranges::range_reference_t<T>>, true>() } };
        } else if constexpr (Concepts::InstanceOfA<T, TypedBoundApiFunction>) {
            return { { TypeEnum::BoundApiFunctionValue }, Concepts::is_instance_auto<T, TypedBoundApiFunction>::arguments::value };
        } else if constexpr (Concepts::Pointer<T>) {
            if constexpr (Concepts::Function<std::remove_pointer_t<T>>)
                // return { { ValueTypeEnum::ApiFunctionValue }, nullptr };
                throw 0;
            else if constexpr (std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, FunctionTable>)
                // return { { ValueTypeEnum::ApiFunctionValue }, nullptr };
                throw 0;
            else {
                return { { TypeEnum::ScopeValue }, &table<std::remove_pointer_t<resolveCustomScopePtr_t<T>>> };
            }
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, std::unique_ptr>) {
            return { { TypeEnum::ScopeValue }, &table<typename Concepts::is_instance<std::decay_t<T>, std::unique_ptr>::argument_types::first> };
        } else {
            return { { TypeEnum::OwnedScopeValue }, &table<meta_decayed_t<resolveCustomScopePtr_t<T>>> };
        }
    }

}
}