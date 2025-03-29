#pragma once

#include "Meta/keyvalue/ownedscopeptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyOwnedScopePtrType;

        struct PyOwnedScopePtr {
            PyObject_HEAD
                OwnedScopePtr mPtr;
        };

    }
}
}
