#include "../python3lib.h"

#include "pyownedvalue.h"

#include "Meta/reflect/scopeiterator.h"
#include "Meta/reflect/value.h"

#include "pyobjectptr.h"
#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *PyOwnedValue_get(PyOwnedValue *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            Reflect::Value v;
            self->mValue.get(v);
            PyObjectPtr inner = toPyObject(v);
            return inner.get(name).release();
        }

        static PyObject *PyOwnedValue_iter(PyOwnedValue *self)
        {
            Reflect::Value v;
            self->mValue.get(v);
            PyObjectPtr inner = toPyObject(v);
            return PyObject_GetIter(inner);
        }

        static PyObject *
        PyOwnedValue_str(PyOwnedValue *self)
        {
            return PyUnicode_FromString(self->mValue.name().c_str());
        }

        PyTypeObject PyOwnedValueType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.OwnedValue",
            .tp_basicsize = sizeof(PyOwnedValue),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyOwnedValue, &PyOwnedValue::mValue>,
            .tp_str = (reprfunc)PyOwnedValue_str,
            .tp_getattro = (getattrofunc)PyOwnedValue_get,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of OwnedValue",
            .tp_iter = (getiterfunc)PyOwnedValue_iter,
            .tp_new = PyType_GenericNew,
        };

    }
}
}