#pragma once

#include "Meta/enumholder.h"

namespace Engine {
namespace Behavior{
    namespace Python3 {

        extern PyTypeObject PyEnumType;

        struct PyEnum {
            PyObject_HEAD
                EnumHolder mEnum;
        };

    }
}
}
