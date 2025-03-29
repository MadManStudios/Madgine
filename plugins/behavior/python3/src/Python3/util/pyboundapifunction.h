#pragma once

#include "Meta/keyvalue/boundapifunction.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyBoundApiFunctionType;

        struct PyBoundApiFunction {
            PyObject_HEAD
                BoundApiFunction mFunction;
        };

    }
}
}
