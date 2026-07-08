#include "../python3lib.h"

#include "pytype.h"

#include "Meta/reflect/argumentlist.h"
#include "Meta/reflect/metatable.h"
#include "Meta/reflect/ownedscopeptr.h"
#include "Meta/reflect/scopeiterator.h"
#include "Meta/reflect/value.h"

#include "pydictptr.h"
#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *PyType_str(PyType *self)
        {
            return PyUnicode_FromString(self->mType->mTypeName);
        }

        static PyObject *PyType_call(PyType *self, PyObject *args, PyObject *kwargs)
        {
            if (!self->mType->mConstructors[0].mMatcher) {
                PyErr_Format(PyExc_TypeError, "%R is not constructible", self);
                return nullptr;
            }

            assert(PyTuple_Check(args));

            size_t argCount = PyTuple_Size(args);
            Reflect::ArgumentList arguments { std::true_type {}, argCount };

            for (size_t i = 0; i < argCount; ++i) {
                PYTHON3_PROPAGATE_ERROR(fromPyObject(arguments[i], PyTuple_GetItem(args, i)));
            }

            Reflect::OwnedScopePtr scope;
            Reflect::Result result = scope.construct(self->mType, arguments);
            if (result) {
                return toPyError(*result.mError);
            }

            if (kwargs) {
                PyDictPtr dict = PyDictPtr::fromBorrowed(kwargs);

                for (auto [key, value] : dict) {
                    assert(PyUnicode_Check(key));

                    PyObjectPtr ascii_key = PyUnicode_AsASCIIString(key);

                    const char *name = PyBytes_AsString(ascii_key);

                    Reflect::ScopeIterator it = self->mType->find(name, Reflect::Value { scope });
                    if (it == it.end()) {
                        PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
                        return nullptr;
                    }

                    Reflect::Value reflectValue;
                    result = fromPyObject(reflectValue, value);
                    if (result) {
                        return toPyError(*result.mError);
                    }

                    result = (*it = reflectValue);
                    if (result) {
                        return toPyError(*result.mError);
                    }
                }
            }

            return toPyObject(std::move(scope));
        }

        PyTypeObject PyTypeType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.MetaTable",
            .tp_basicsize = sizeof(PyType),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyType, &PyType::mType>,
            .tp_repr = (reprfunc)PyType_str,
            .tp_call = (ternaryfunc)PyType_call,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of MetaTable",
            .tp_new = PyType_GenericNew,
        };

    }
}
}