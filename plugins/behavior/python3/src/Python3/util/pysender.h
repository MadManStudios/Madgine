#pragma once

#include "Generic/execution/stoppable.h"

#include "Meta/reflect/sender.h"

#include "Madgine/behavior/behaviorreceiver.h"
#include "Madgine/debug/debuggablesender.h"

#include "pyexecution.h"
#include "pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PySenderType;

        struct PySender {
            PyObject_HEAD
                Reflect::Sender mSender;
        };

    }
}
}
