#pragma once

#include "valuetype_desc.h"

namespace Engine {


struct Accessor {
    const char *mName = nullptr;
    bool (*mCheck)(const Accessor *self, const ScopePtr &) = nullptr;
    void (*mGetter)(const Accessor *self, ValueType &, const ScopePtr &) = nullptr;
    void (*mSetter)(const Accessor *self, const ScopePtr &, const ValueType &) = nullptr;
    ExtendedValueTypeDesc mType { ExtendedValueTypeEnum::GenericType };
};

}