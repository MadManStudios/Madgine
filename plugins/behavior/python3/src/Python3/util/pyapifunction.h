#pragma once

#include "Meta/keyvalue/apifunction.h"

namespace Engine {
namespace Behavior{
    namespace Python3 {

        extern PyTypeObject PyApiFunctionType;

        struct PyApiFunction {
            PyObject_HEAD
                ApiFunction mFunction;
        };

    }
}
}
