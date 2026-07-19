#pragma once

#include "Meta/reflect/ownedvalue.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyOwnedValueType;

        struct PyOwnedValue {
            PyObject_HEAD
                Reflect::OwnedValue mValue;
        };

    }
}
}
