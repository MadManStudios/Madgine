#include "../python3lib.h"

#include "pyflags.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyFlagsType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.Flags",
            .tp_basicsize = sizeof(PyFlags),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyFlags, &PyFlags::mFlags>,
            .tp_str = &PyStr<PyFlags, &PyFlags::mFlags>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of FlagsHolder",
            .tp_new = PyType_GenericNew,
        };

    }
}
}