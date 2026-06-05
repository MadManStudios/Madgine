#pragma once

#include "Meta/reflect/binding.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyBindingType;

        struct PyBinding {
            PyObject_HEAD
                Reflect::Binding mBinding;
        };

        extern PyTypeObject PyScopeBindingType;

        struct PyScopeBinding {
            PyObject_HEAD
                Reflect::ScopeBinding mBinding;
        };

    }
}
}
