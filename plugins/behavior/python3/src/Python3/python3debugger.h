#pragma once

#include "Madgine/debug/debugger.h"

#include "util/pyexecution.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3DebugLocation : Debug::DebugLocation {            

            std::string toString() const override;
            std::map<std::string_view, ValueType> localVariables() const override;
            bool wantsPause(Debug::ContinuationType type) const override;

            Filesystem::Path file() const;
            std::string module() const;
            size_t lineNr() const;

            PyFrameObject *mFrame = nullptr;
        };

        struct Python3Debugger {

            struct Guard {
                Guard(Debug::ParentLocation *parent);
                Guard(PyObjectPtr location);
                ~Guard();
            };

            static int trace(PyObject *obj, PyFrameObject *frame, int event, PyObject *arg);

        private:
        };

    }
}
}