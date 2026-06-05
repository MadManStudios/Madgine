#pragma once

#include "Meta/reflect/scopeiterator.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyScopeIteratorType;

        struct PyScopeIterator {
            PyObject_HEAD
                Reflect::ScopeIterator mIt;
        };

    }
}
}
