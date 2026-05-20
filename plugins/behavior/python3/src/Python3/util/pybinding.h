#pragma once

#include "Meta/keyvalue/keyvaluebinding.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyBindingType;

        struct PyBinding {
            PyObject_HEAD
                KeyValueBinding mBinding;
        };

        extern PyTypeObject PyScopeBindingType;

        struct PyScopeBinding {
            PyObject_HEAD
                KeyValueScopeBinding mBinding;
        };

    }
}
}
