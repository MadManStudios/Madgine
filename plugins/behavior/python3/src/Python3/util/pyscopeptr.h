#pragma once

#include "Meta/keyvalue/scopeptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyTypedScopePtrType;

        struct PyTypedScopePtr {
            PyObject_HEAD
                ScopePtr mPtr;
        };

    }
}
}
