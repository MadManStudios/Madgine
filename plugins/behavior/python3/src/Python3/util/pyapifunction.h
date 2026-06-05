#pragma once

#include "Meta/reflect/apifunction.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyApiFunctionType;

        struct PyApiFunction {
            PyObject_HEAD
                Reflect::ApiFunction mFunction;
        };

    }
}
}
