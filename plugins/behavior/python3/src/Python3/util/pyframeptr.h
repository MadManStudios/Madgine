#pragma once

#include "pyobjectptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT PyFramePtr : PyObjectPtr {
            PyFramePtr() = default;
            PyFramePtr(_PyInterpreterFrame *frame);

            static PyFramePtr fromBorrowed(_PyInterpreterFrame *frame);

            using PyObjectPtr::operator=;

            operator _PyInterpreterFrame *() const;
            _PyInterpreterFrame *operator->() const;

            _PyInterpreterFrame *release();

        };

    
    }
}
}