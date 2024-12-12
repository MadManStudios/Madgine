#include "../python3lib.h"

#include "pysender.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PySenderType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.Sender",
            .tp_basicsize = sizeof(PySender),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PySender, &PySender::mSender>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueSender",
            .tp_new = PyType_GenericNew,
        };

    }
}
}