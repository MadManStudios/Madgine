#pragma once

#include "Meta/keyvalue/scopeiterator.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyScopeIteratorType;

        struct PyScopeIterator {
            PyObject_HEAD
                ScopeIterator mIt;
        };

    }
}
}
