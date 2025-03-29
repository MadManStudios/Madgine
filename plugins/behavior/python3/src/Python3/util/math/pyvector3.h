#pragma once

#include "Meta/math/vector3.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyVector3Type;

        struct PyVector3 {
            PyObject_HEAD
                Vector3 mVector;
        };

    }
}
}
