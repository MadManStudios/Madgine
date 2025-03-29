#include "../python3lib.h"

#include "pyvirtualiterator.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/valuetype.h"

#include "Meta/keyvalue/keyvaluepair.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        static PyObject *
        PyVirtualSequenceIterator_iter(PyObject *self)
        {
            Py_INCREF(self);
            return self;
        }

        static PyObject *
        PyVirtualSequenceIterator_next(PyVirtualSequenceIterator *self)
        {
            if (self->mIt.ended())
                return NULL;
            PyObject *result = toPyObject(*self->mIt);
            ++self->mIt;
            return result;
        }

        PyTypeObject PyVirtualSequenceIteratorType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.VirtualSequenceIterator",
            .tp_basicsize = sizeof(PyVirtualSequenceIterator),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVirtualSequenceIterator, &PyVirtualSequenceIterator::mIt>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of VirtualSequenceIterator",
            .tp_iter = (getiterfunc)PyVirtualSequenceIterator_iter,
            .tp_iternext = (iternextfunc)PyVirtualSequenceIterator_next,
            .tp_new = PyType_GenericNew,
        };

        static PyObject *
        PyVirtualAssociativeIterator_iter(PyObject *self)
        {
            Py_INCREF(self);
            return self;
        }

        static PyObject *
        PyVirtualAssociativeIterator_next(PyVirtualAssociativeIterator *self)
        {
            if (self->mIt.ended())
                return NULL;
            PyObject *result = Py_BuildValue("NN", toPyObject(self->mIt->mKey), toPyObject(self->mIt->mValue));
            ++self->mIt;
            return result;
        }

        PyTypeObject PyVirtualAssociativeIteratorType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.VirtualAssociativeIterator",
            .tp_basicsize = sizeof(PyVirtualAssociativeIterator),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVirtualAssociativeIterator, &PyVirtualAssociativeIterator::mIt>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of VirtualAssociativeIterator",
            .tp_iter = (getiterfunc)PyVirtualAssociativeIterator_iter,
            .tp_iternext = (iternextfunc)PyVirtualAssociativeIterator_next,
            .tp_new = PyType_GenericNew,
        };

    }
}
}