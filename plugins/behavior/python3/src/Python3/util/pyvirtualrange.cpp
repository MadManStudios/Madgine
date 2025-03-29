#include "../python3lib.h"

#include "pyvirtualrange.h"

#include "Meta/keyvalue/valuetype.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/keyvaluepair.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        static PyObject *
        PyVirtualSequenceRange_iter(PyVirtualSequenceRange *self)
        {
            return toPyObject(self->mRange.begin());
        }

        PyTypeObject PyVirtualSequenceRangeType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.VirtualSequenceRange",
            .tp_basicsize = sizeof(PyVirtualSequenceRange),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVirtualSequenceRange, &PyVirtualSequenceRange::mRange>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueVirtualSequenceRange",
            .tp_iter = (getiterfunc)PyVirtualSequenceRange_iter,
            .tp_new = PyType_GenericNew,
        };

        static PyObject *
        PyVirtualAssociativeRange_iter(PyVirtualAssociativeRange *self)
        {
            return toPyObject(self->mRange.begin());
        }

        static PyObject *
        PyVirtualAssociativeRange_subscript(PyVirtualAssociativeRange *self, PyObject *key)
        {
            ValueType val = fromPyObject(key);

            auto it = std::ranges::find(self->mRange, val, &KeyValuePair::mKey);
            if (it == self->mRange.end()) {
                PyErr_Format(PyExc_KeyError, "No key %R in %R!", key, self);
                return NULL;
            }
            return toPyObject(it->mValue);
        }

        static PyMappingMethods PyVirtualAssociativeRangeMapping {
            .mp_subscript = (binaryfunc)&PyVirtualAssociativeRange_subscript,
        };

        PyTypeObject PyVirtualAssociativeRangeType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.VirtualAssociativeRange",
            .tp_basicsize = sizeof(PyVirtualAssociativeRange),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVirtualAssociativeRange, &PyVirtualAssociativeRange::mRange>,
            .tp_as_mapping = &PyVirtualAssociativeRangeMapping,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueVirtualAssociativeRange",
            .tp_iter = (getiterfunc)PyVirtualAssociativeRange_iter,
            .tp_new = PyType_GenericNew
        };

    }
}
}