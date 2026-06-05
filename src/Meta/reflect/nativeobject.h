#pragma once

#include "objectptr.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT NativeObject : ObjectPtr {

        NativeObject(std::map<std::string, Value, std::less<>> data);
    };

}
}