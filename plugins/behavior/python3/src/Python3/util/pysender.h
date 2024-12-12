#pragma once

#include "Meta/keyvalue/keyvaluesender.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PySenderType;

        struct PySender {
            PyObject_HEAD
                KeyValueSender mSender;
        };

    }
}
}
