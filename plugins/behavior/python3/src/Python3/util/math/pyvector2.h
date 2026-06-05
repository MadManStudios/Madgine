#pragma once

#include "Meta/math/vector2.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyVector2Type;

        struct PyVector2 {
            PyObject_HEAD
                Math::Vector2 mVector;
        };

    }
}
}
