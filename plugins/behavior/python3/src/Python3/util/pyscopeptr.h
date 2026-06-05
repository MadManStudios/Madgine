#pragma once

#include "Meta/reflect/scopeptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyTypedScopePtrType;

        struct PyTypedScopePtr {
            PyObject_HEAD
                Reflect::ScopePtr mPtr;
        };

    }
}
}
