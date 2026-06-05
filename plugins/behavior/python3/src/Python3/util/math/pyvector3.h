#pragma once

#include "Meta/math/vector3.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyVector3Type;

        struct PyVector3 {
            PyObject_HEAD
                Math::Vector3 mVector;
        };

    }
}
}
