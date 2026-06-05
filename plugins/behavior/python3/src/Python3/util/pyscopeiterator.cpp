#include "../python3lib.h"

#include "pyscopeiterator.h"

#include "Meta/reflect/value.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *
        PyScopeIterator_iter(PyObject *self)
        {
            Py_INCREF(self);
            return self;
        }

        static PyObject *
        PyScopeIterator_next(PyScopeIterator *self)
        {
            if (self->mIt == self->mIt.end())
                return NULL;
            Reflect::Value v;
            PYTHON3_PROPAGATE_ERROR(self->mIt->value(v));
            PyObject *item = toPyObject(v);
            if (!item)
                return NULL;
            PyObject *result = Py_BuildValue("sN", self->mIt->key(), item);
            ++self->mIt;
            return result;
        }

        PyTypeObject PyScopeIteratorType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.ScopeIterator",
            .tp_basicsize = sizeof(PyScopeIterator),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyScopeIterator, &PyScopeIterator::mIt>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of ScopeIterator",
            .tp_iter = (getiterfunc)PyScopeIterator_iter,
            .tp_iternext = (iternextfunc)PyScopeIterator_next,
            .tp_new = PyType_GenericNew,
        };

    }
}
}