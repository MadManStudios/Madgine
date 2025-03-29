#include "../../python3lib.h"

#include "pyquaternion.h"

#include "../pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyQuaternionType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Quaternion",
            .tp_basicsize = sizeof(PyQuaternion),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyQuaternion, &PyQuaternion::mQuaternion>,
            .tp_str = &PyStr<PyQuaternion, &PyQuaternion::mQuaternion>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Quaternion",
            .tp_new = PyType_GenericNew,
        };

    }
}
}