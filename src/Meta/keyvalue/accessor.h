#pragma once

#include "valuetype_desc.h"

namespace Engine {


struct Accessor {
    void (*mGetter)(ValueType &, const ScopePtr &);
    void (*mSetter)(const ScopePtr &, const ValueType &);
    ExtendedValueTypeDesc mType;
};

}