#include "../../python3lib.h"

#include "pyvector4.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyVector4Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Vector4",
            .tp_basicsize = sizeof(PyVector4),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVector4, &PyVector4::mVector>,
            .tp_str = &PyStr<PyVector4, &PyVector4::mVector>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Vector4",
            .tp_new = PyType_GenericNew,
        };

    }
}
}