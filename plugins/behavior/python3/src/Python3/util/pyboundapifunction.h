#pragma once

#include "Meta/reflect/boundapifunction.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyBoundApiFunctionType;

        struct PyBoundApiFunction {
            PyObject_HEAD
                Reflect::BoundApiFunction mFunction;
        };

    }
}
}
