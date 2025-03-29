#include "../../python3lib.h"

#include "pymatrix3.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyMatrix3Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Matrix3",
            .tp_basicsize = sizeof(PyMatrix3),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyMatrix3, &PyMatrix3::mMatrix>,
            .tp_str = &PyStr<PyMatrix3, &PyMatrix3::mMatrix>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Matrix3",
            .tp_new = PyType_GenericNew,
        };

    }
}
}