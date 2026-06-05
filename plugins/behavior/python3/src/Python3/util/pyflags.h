#pragma once

#include "Meta/reflect/flags.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyFlagsType;

        struct PyFlags {
            PyObject_HEAD
                Reflect::Flags mFlags;
        };

    }
}
}
