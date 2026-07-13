#include "../python3lib.h"

#include "pysender.h"

#include "Madgine/behavior/behaviorreceiver.h"

#include "pyobjectptr.h"
#include "pyobjectutil.h"
#include "python3lock.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *PySender_await(PyObject *self)
        {
            Reflect::Sender sender = reinterpret_cast<PySender *>(self)->mSender;

            return PyAwait(std::move(sender));
        }

        PyAsyncMethods PySenderAsyncMethods = {
            .am_await = PySender_await
        };

        PyTypeObject PySenderType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Sender",
            .tp_basicsize = sizeof(PySender),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PySender, &PySender::mSender>,
            .tp_as_async = &PySenderAsyncMethods,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Reflect::Sender",
            .tp_new = PyType_GenericNew,
        };

    }
}
}