#pragma once

#include "Meta/reflect/enum.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyEnumType;

        struct PyEnum {
            PyObject_HEAD
                Reflect::Enum mEnum;
        };

    }
}
}
