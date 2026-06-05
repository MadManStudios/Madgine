#pragma once

#include "Meta/math/vector4.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyVector4Type;

        struct PyVector4 {
            PyObject_HEAD
                Math::Vector4 mVector;
        };

    }
}
}
