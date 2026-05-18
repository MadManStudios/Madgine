#pragma once

#include "valuetype_desc.h"

namespace Engine {

    using AccessorFlags = uint32_t;

    enum AccessorFlags_ {
        AccessorFlags_Named = (1 << 0),
        AccessorFlags_Default = 0
    };

struct Accessor {
    const char *mName = nullptr;
    bool (*mCheck)(const Accessor *self, const ValueType &) = nullptr;
    KeyValueResult (*mGetter)(const Accessor *self, ValueType &, const ValueType &) = nullptr;
    KeyValueResult (*mSetter)(const Accessor *self, const ValueType &, const ValueType &) = nullptr;
    ExtendedValueTypeDesc mType { ExtendedValueTypeEnum::GenericType };
    AccessorFlags mFlags = AccessorFlags_Default;

};

}