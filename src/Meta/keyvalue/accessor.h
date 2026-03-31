#pragma once

#include "valuetype_desc.h"

namespace Engine {

struct Accessor {
    const char *mName = nullptr;
    bool (*mCheck)(const Accessor *self, const ScopePtr &) = nullptr;
    KeyValueResult (*mGetter)(const Accessor *self, ValueType &, const ValueType &) = nullptr;
    KeyValueResult (*mSetter)(const Accessor *self, const ValueType &, const ValueType &) = nullptr;
    ExtendedValueTypeDesc mType { ExtendedValueTypeEnum::GenericType };
};

}