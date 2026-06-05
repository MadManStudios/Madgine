#pragma once

#include "Meta/math/quaternion.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyQuaternionType;

        struct PyQuaternion {
            PyObject_HEAD
                Math::Quaternion mQuaternion;
        };

    }
}
}
