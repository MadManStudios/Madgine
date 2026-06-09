#pragma once

#include "Meta/reflect/type.h"

namespace Engine {
namespace Behavior {

    struct NamedDescriptor {
        std::string mName;
        Reflect::ExtendedType mType;
    };

}
}