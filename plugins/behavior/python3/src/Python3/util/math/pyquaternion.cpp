#include "../../python3lib.h"

#include "pyquaternion.h"

#include "../pyobjectutil.h"
#include "pyvector3.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *PyQuaternion_multiply(PyQuaternion *q, PyObject *other)
        {
            if (PyObject_TypeCheck(other, &PyVector3Type)) {
                return toPyObject(q->mQuaternion * ((PyVector3 *)other)->mVector);
            } else {
                return Py_NotImplemented;
            }
        }

        PyNumberMethods PyQuaternionNumberMethods = {
            .nb_multiply = (binaryfunc)&PyQuaternion_multiply,
        };

        PyTypeObject PyQuaternionType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Quaternion",
            .tp_basicsize = sizeof(PyQuaternion),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyQuaternion, &PyQuaternion::mQuaternion>,
            .tp_as_number = &PyQuaternionNumberMethods,
            .tp_str = &PyStr<PyQuaternion, &PyQuaternion::mQuaternion>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Quaternion",
            .tp_new = PyType_GenericNew,
        };

    }
}
}