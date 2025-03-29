#include "../../python3lib.h"

#include "pyvector3.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyVector3Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Vector3",
            .tp_basicsize = sizeof(PyVector3),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVector3, &PyVector3::mVector>,
            .tp_str = &PyStr<PyVector3, &PyVector3::mVector>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Vector3",
            .tp_new = PyType_GenericNew,
        };

    }
}
}