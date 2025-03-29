#include "../../python3lib.h"

#include "pymatrix4.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyMatrix4Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Matrix4",
            .tp_basicsize = sizeof(PyMatrix4),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyMatrix4, &PyMatrix4::mMatrix>,
            .tp_str = &PyStr<PyMatrix4, &PyMatrix4::mMatrix>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Matrix4",
            .tp_new = PyType_GenericNew,
        };

    }
}
}