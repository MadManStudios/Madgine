#pragma once

#include "Meta/math/matrix3.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyMatrix3Type;

        struct PyMatrix3 {
            PyObject_HEAD
                Matrix3 mMatrix;
        };

    }
}
}
