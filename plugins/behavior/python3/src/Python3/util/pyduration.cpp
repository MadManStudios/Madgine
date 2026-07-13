#include "../python3lib.h"

#include "pyduration.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {
        
        PyNumberMethods PyDurationNumberMethods {

        };

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
            .tp_new = PyType_GenericNew,
        };

    }
}
}