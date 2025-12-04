#pragma once

#include "valuetype_desc.h"

namespace Engine {

struct FunctionArgument {
    ExtendedValueTypeDesc mType;
    std::string_view mName;
};

}