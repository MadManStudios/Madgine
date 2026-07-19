#include "../metalib.h"

#include "type.h"

#include "metatable.h"

namespace Engine {
namespace Reflect {

    std::string_view TypeIndex::toString() const
    {
        switch (mIndex) {
#define VALUE_SEP
#define VALUE_TYPE(Name, Storage, ...) \
    case TypeEnum::Name##Value:       \
        return #Name;
#include "valuedefinclude.h"
        default:
            std::terminate();
        }
    }

    std::string_view TypeIndex::toTypeName() const
    {
        switch (mIndex) {
#define VALUE_SEP
#define VALUE_TYPE(Name, Storage, ...) \
    case TypeEnum::Name##Value:       \
        return #Storage;
#include "valuedefinclude.h"
        default:
            std::terminate();
        }
    }

    bool Type::canAccept(const Type &valueType)
    {
        if (mType != valueType.mType)
            return false;
        switch (mType) {
        case TypeEnum::ScopeValue:
            if (!mSecondary.mMetaTable)
                return true;
            return (*valueType.mSecondary.mMetaTable)->isDerivedFrom(*mSecondary.mMetaTable);
        default:
            return true;
        }
    }

    std::string Type::toString() const
    {
        switch (mType) {
        case TypeEnum::ScopeValue:
            return mSecondary.mMetaTable ? std::string { (*mSecondary.mMetaTable)->mTypeName } + "*" : "<no-type>";
        case TypeEnum::OwnedValueValue:
            return (*mSecondary.mMetaTable)->mTypeName;
        default:
            return std::string { mType.toString() };
        }
    }

    std::string Type::toTypeName() const
    {
        switch (mType) {
        case TypeEnum::ScopeValue:
            return mSecondary.mMetaTable ? std::string { (*mSecondary.mMetaTable)->mTypeName } + "*" : "<no-type>";
        case TypeEnum::OwnedValueValue:
            return (*mSecondary.mMetaTable)->mTypeName;
        default:
            return std::string { mType.toTypeName() };
        }
    }

    std::string_view ExtendedTypeIndex::toString(size_t level) const
    {
        if (isRegular(level)) {
            return TypeIndex { static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mTypeList[level])) }.toString();
        } else {
            switch (mTypeList[level]) {
            case ExtendedTypeEnum::GenericType:
                return "Engine::ValueType";
            default:
                throw 0;
            }
        }
    }

    std::string ExtendedType::toString(size_t level) const
    {
        if (mType.mTypeList[level] == static_cast<ExtendedTypeEnum>(TypeEnum::SequenceRangeValue)) {
            return "Range<" + toString(level + 1) + ">";
        } else if (mType.mTypeList[level] == static_cast<ExtendedTypeEnum>(TypeEnum::AssociativeRangeValue)) {
            return "Map<" + toString(level + 1) + ", " + toString(level + 2) + ">";
        } else if (mType.mTypeList[level] == ExtendedTypeEnum::VariantType) {
            return "Variant<" + toString(level + 1) + ", " + toString(level + 2) + ">";
        } else if (mType.mTypeList[level] == static_cast<ExtendedTypeEnum>(TypeEnum::BindingValue)) {
            return "Binding<" + toString(level + 1) + ">";
        } else if (mType.isRegular(level)) {
            return Type { { static_cast<TypeEnum>(static_cast<ExtendedTypeEnum>(mType.mTypeList[level])) }, mSecondary }.toString();
        } else {
            return std::string { mType.toString(level) };
        }
    }

}
}