#pragma once

#include "objectptr.h"

namespace Engine {

struct META_EXPORT NativeObject : ObjectPtr {

    NativeObject(std::map<std::string, ValueType, std::less<>> data);
};

}