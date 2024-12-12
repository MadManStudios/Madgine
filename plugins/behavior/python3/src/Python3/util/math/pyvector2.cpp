#include "../../python3lib.h"

#include "pyvector2.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyVector2Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Vector2",
            .tp_basicsize = sizeof(PyVector2),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVector2, &PyVector2::mVector>,
            .tp_str = &PyStr<PyVector2, &PyVector2::mVector>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Vector2",
            .tp_new = PyType_GenericNew,
        };

    }
}
}