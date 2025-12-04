#include "../python3lib.h"

#include "pyframeptr.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyFramePtr::PyFramePtr(PyFrameObject *frame)
            : PyObjectPtr((PyObject *)frame)
        {
        }

        PyFramePtr PyFramePtr::fromBorrowed(PyFrameObject *frame)
        {
            Py_INCREF(frame);
            return frame;
        }

        PyFramePtr::operator PyFrameObject *() const
        {
            return (PyFrameObject *)(static_cast<PyObject *>(*this));
        }

        PyFrameObject *PyFramePtr::operator->() const
        {
            return (PyFrameObject *)(static_cast<PyObject *>(*this));
        }

        PyFrameObject *PyFramePtr::release()
        {
            return (PyFrameObject *)PyObjectPtr::release();
        }

    }
}
}