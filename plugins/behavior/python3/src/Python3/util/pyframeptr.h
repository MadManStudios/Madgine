#pragma once

#include "pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT PyFramePtr : PyObjectPtr {
            PyFramePtr() = default;
            PyFramePtr(PyFrameObject *frame);

            static PyFramePtr fromBorrowed(PyFrameObject *frame);

            using PyObjectPtr::operator=;

            operator PyFrameObject *() const;
            PyFrameObject *operator->() const;

            PyFrameObject *release();
        };

    }
}
}