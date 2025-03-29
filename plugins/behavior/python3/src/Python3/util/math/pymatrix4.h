#pragma once

#include "Meta/math/matrix4.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyMatrix4Type;

        struct PyMatrix4 {
            PyObject_HEAD
                Matrix4 mMatrix;
        };

    }
}
}
