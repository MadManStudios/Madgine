#include "../metalib.h"

#include "valuetype_desc.h"

#include "metatable.h"

namespace Engine {

std::string_view ValueTypeIndex::toString() const
{
    switch (mIndex) {
#define VALUETYPE_SEP
#define VALUETYPE_TYPE(Name, Storage, ...) \
    case ValueTypeEnum::Name##Value:       \
        return #Name;
#include "valuetypedefinclude.h"
    default:
        std::terminate();
    }
}

std::string_view ValueTypeIndex::toTypeName() const
{
    switch (mIndex) {
#define VALUETYPE_SEP
#define VALUETYPE_TYPE(Name, Storage, ...) \
    case ValueTypeEnum::Name##Value:       \
        return #Storage;
#include "valuetypedefinclude.h"
    default:
        std::terminate();
    }
}

bool ValueTypeDesc::canAccept(const ValueTypeDesc &valueType)
{
    if (mType != valueType.mType)
        return false;
    switch (mType) {
    case ValueTypeEnum::ScopeValue:
        if (!mSecondary.mMetaTable)
            return true;
        return (*valueType.mSecondary.mMetaTable)->isDerivedFrom(*mSecondary.mMetaTable);
    default:
        return true;
    }
}

std::string ValueTypeDesc::toString() const
{
    switch (mType) {
    case ValueTypeEnum::ScopeValue:
        return mSecondary.mMetaTable ? std::string { (*mSecondary.mMetaTable)->mTypeName } + "*" : "<no-type>";
    case ValueTypeEnum::OwnedScopeValue:
        return (*mSecondary.mMetaTable)->mTypeName;
    default:
        return std::string { mType.toString() };
    }
}

std::string ValueTypeDesc::toTypeName() const
{
    switch (mType) {
    case ValueTypeEnum::ScopeValue:
        return mSecondary.mMetaTable ? std::string { (*mSecondary.mMetaTable)->mTypeName } + "*" : "<no-type>";
    case ValueTypeEnum::OwnedScopeValue:
        return (*mSecondary.mMetaTable)->mTypeName;
    default:
        return std::string { mType.toTypeName() };
    }
}

std::string_view ExtendedValueTypeIndex::toString(size_t level) const
{
    if (isRegular(level)) {
        return ValueTypeIndex { static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mTypeList[level])) }.toString();
    } else {
        switch (mTypeList[level]) {
        case ExtendedValueTypeEnum::GenericType:
            return "Engine::ValueType";
        default:
            throw 0;
        }
    }
}

std::string ExtendedValueTypeDesc::toString(size_t level) const
{
    if (mType.mTypeList[level] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualSequenceRangeValue)) {
        return "Range<" + toString(level + 1) + ">";
    } else if (mType.mTypeList[level] == static_cast<ExtendedValueTypeEnum>(ValueTypeEnum::KeyValueVirtualAssociativeRangeValue)) {
        return "Map<" + toString(level + 1) + ", " + toString(level + 2) + ">";
    } else if (mType.isRegular(level)) {
        return ValueTypeDesc { { static_cast<ValueTypeEnum>(static_cast<ExtendedValueTypeEnum>(mType.mTypeList[level])) }, mSecondary }.toString();
    } else {
        return std::string { mType.toString(level) };
    }
}

}