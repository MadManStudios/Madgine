#pragma once

#include "Madgine/debug/debuglocation.h"

#include "util/pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3DebugLocation {            

            Filesystem::Path file() const;
            std::string module() const;
            size_t lineNr() const;

            PyFrameObject *mFrame = nullptr;
        };

        struct Python3Debugger {

            struct Guard {
                Guard();
                Guard(PyObjectPtr location);
                ~Guard();
            };

            static int trace(PyObject *obj, PyFrameObject *frame, int event, PyObject *arg);

        private:
        };

    }
}
}