#include "../python3lib.h"

#include "pyboundapifunction.h"

#include "Meta/keyvalue/valuetype.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/functiontable.h"

#include "pyexecution.h"

#include "python3lock.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        static PyObject *
        PyBoundApiFunction_call(PyBoundApiFunction *self, PyObject *args, PyObject *kwargs)
        {
            size_t argCount = PyTuple_Size(args);
            ArgumentList arguments { argCount };

            for (size_t i = 0; i < argCount; ++i) {
                arguments[i] = fromPyObject(PyTuple_GetItem(args, i));
            }

            ValueType retVal;
            Py_BEGIN_ALLOW_THREADS
                self->mFunction(retVal, arguments);
            Py_END_ALLOW_THREADS

                return toPyObject(retVal);
        }

        PyTypeObject PyBoundApiFunctionType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.BoundApiFunction",
            .tp_basicsize = sizeof(PyBoundApiFunction),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyBoundApiFunction, &PyBoundApiFunction::mFunction>,
            .tp_call = (ternaryfunc)PyBoundApiFunction_call,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of BoundApiFunction",
            .tp_new = PyType_GenericNew,
        };

    }
}
}