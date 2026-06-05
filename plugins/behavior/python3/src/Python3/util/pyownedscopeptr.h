#pragma once

#include "Meta/reflect/ownedscopeptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyOwnedScopePtrType;

        struct PyOwnedScopePtr {
            PyObject_HEAD
                Reflect::OwnedScopePtr mPtr;
        };

    }
}
}
