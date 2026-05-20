#include "../python3lib.h"

#include "pytype.h"

#include "Meta/keyvalue/metatable.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *PyType_str(PyType *self)
        {
            return PyUnicode_FromString(self->mType->mTypeName);
        }

        PyTypeObject PyTypeType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.MetaTable",
            .tp_basicsize = sizeof(PyType),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyType, &PyType::mType>,
            .tp_repr = (reprfunc)PyType_str,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of MetaTable",
            .tp_new = PyType_GenericNew,
        };

    }
}
}