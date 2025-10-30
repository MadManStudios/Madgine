#include "../python3lib.h"

#include "pyframeptr.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Scripting::Python3::PyFramePtr)
METATABLE_END(Engine::Scripting::Python3::PyFramePtr)

namespace Engine {
namespace Scripting {
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