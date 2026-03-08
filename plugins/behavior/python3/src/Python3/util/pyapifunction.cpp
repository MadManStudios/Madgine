#include "../python3lib.h"

#include "pyapifunction.h"

#include "Meta/keyvalue/functiontable.h"
#include "Meta/keyvalue/valuetype.h"

#include "../python3env.h"
#include "pyexecution.h"
#include "pyobjectutil.h"
#include "python3lock.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *
        PyApiFunction_call(PyApiFunction *self, PyObject *args, PyObject *kwargs)
        {
            size_t argCount = PyTuple_Size(args);
            ArgumentList arguments { argCount };

            for (size_t i = 0; i < argCount; ++i) {
                PYTHON3_PROPAGATE_ERROR(fromPyObject(arguments[i], PyTuple_GetItem(args, i)));
            }

            ValueType retVal;
            KeyValueResult result;
            Py_BEGIN_ALLOW_THREADS;
            result = self->mFunction(retVal, arguments);
            Py_END_ALLOW_THREADS;
            PYTHON3_PROPAGATE_ERROR(std::move(result));

            return toPyObject(retVal);
        }

        PyTypeObject PyApiFunctionType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.ApiFunction",
            .tp_basicsize = sizeof(PyApiFunction),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyApiFunction, &PyApiFunction::mFunction>,
            .tp_call = (ternaryfunc)PyApiFunction_call,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of ApiFunction",
            .tp_new = PyType_GenericNew,
        };

    }
}
}