#include "../python3lib.h"

#include "pyduration.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {
        
        PyNumberMethods PyDurationNumberMethods {

        };

        int PyDuration_init(PyDuration* self, PyObject* args, PyObject* kwds) {
            unsigned long long value;
            if (!PyArg_ParseTuple(args, "K", &value))
                return -1;

            self->mDuration = Reflect::Duration64 { value };
            return 0;
        }

        PyObject* PyDuration_compare(PyObject*, PyObject*, int) {            
            Py_RETURN_NOTIMPLEMENTED;
        }

        PyTypeObject PyDurationType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Duration",
            .tp_basicsize = sizeof(PyDuration),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyDuration, &PyDuration::mDuration>,
            .tp_as_number = &PyDurationNumberMethods,
            .tp_str = &PyStr<PyDuration, &PyDuration::mDuration>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Duration",
            .tp_richcompare = PyDuration_compare,
            .tp_init = (initproc)PyDuration_init,
            .tp_new = PyType_GenericNew,
        };

    }
}
}