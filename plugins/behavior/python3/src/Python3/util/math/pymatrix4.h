#pragma once

#include "Meta/math/matrix4.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyMatrix4Type;

        struct PyMatrix4 {
            PyObject_HEAD
                Math::Matrix4 mMatrix;
        };

    }
}
}
