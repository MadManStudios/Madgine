#include "../python3lib.h"

#include "pyframeptr.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Scripting::Python3::PyFramePtr)
METATABLE_END(Engine::Scripting::Python3::PyFramePtr)

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyFramePtr::PyFramePtr(_PyInterpreterFrame *frame)
            : PyObjectPtr((PyObject *)frame)
        {
        }

        PyFramePtr PyFramePtr::fromBorrowed(_PyInterpreterFrame *frame)
        {
            Py_INCREF(frame);
            return frame;
        }

        PyFramePtr::operator _PyInterpreterFrame *() const
        {
            return (_PyInterpreterFrame *)(static_cast<PyObject *>(*this));
        }

        _PyInterpreterFrame *PyFramePtr::operator->() const
        {
            return (_PyInterpreterFrame *)(static_cast<PyObject *>(*this));
        }

        _PyInterpreterFrame *PyFramePtr::release()
        {
            return (_PyInterpreterFrame *)PyObjectPtr::release();
        }

    }
}
}