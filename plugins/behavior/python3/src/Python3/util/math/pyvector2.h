#pragma once

#include "Meta/math/vector2.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyVector2Type;

        struct PyVector2 {
            PyObject_HEAD
                Vector2 mVector;
        };

    }
}
}
