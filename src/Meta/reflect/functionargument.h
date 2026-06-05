#pragma once

#include "type.h"

namespace Engine {
namespace Reflect {

    struct FunctionArgument {
        ExtendedType mType;
        std::string_view mName;
    };

}
}