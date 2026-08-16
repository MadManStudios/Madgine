#include "../python3lib.h"

#include "pybinding.h"

#include "Meta/reflect/scopeiterator.h"
#include "Meta/reflect/value.h"

#include "pyobjectptr.h"
#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *PyBinding_str(PyBinding *self)
        {
            PyObject *result = nullptr;
            bool success = Execution::access_binding(self->mBinding, [&](const Reflect::Value &v) {
                result = PyUnicode_FromString(v.toShortString().c_str());
            });
            if (!success) {
                PyErr_SetString(PyExc_ReferenceError, "TODO");
            }
            return result;
        }

        static PyObject *PyBinding_await(PyBinding *self)
        {
            PyObject *result = nullptr;
            bool success = Execution::access_binding(self->mBinding, [&](const Reflect::Value &v) {
                result = PyObject_CallFunctionObjArgs(PyObjectPtr { toPyObject(v) }.get("__await__"), NULL);
            });
            if (!success) {
                PyErr_SetString(PyExc_ReferenceError, "TODO");
            }
            return result;
        }

        PyAsyncMethods PyBindingAsyncMethods = {
            .am_await = (unaryfunc)PyBinding_await
        };

        PyTypeObject PyBindingType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Binding",
            .tp_basicsize = sizeof(PyBinding),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyBinding, &PyBinding::mBinding>,
            .tp_as_async = &PyBindingAsyncMethods,
            .tp_repr = (reprfunc)PyBinding_str,
            .tp_str = (reprfunc)PyBinding_str,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueBinding",
            //.tp_iter = (getiterfunc)PyBinding_iter,
            .tp_new = PyType_GenericNew,
        };

        static PyObject *PyScopeBinding_get(PyScopeBinding *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            Reflect::ScopeIterator it = self->mBinding.mType->find(name, Reflect::Value { self->mBinding });
            if (it == Reflect::ScopeIterator { Reflect::Value { self->mBinding }, nullptr }) {
                PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
                return NULL;
            }
            Reflect::Value v;
            PYTHON3_PROPAGATE_ERROR(it->value(v));
            return toPyObject(v);
        }

        static int PyScopeBinding_set(PyScopeBinding *self, PyObject *args, PyObject *value)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            Reflect::ScopeIterator it = self->mBinding.mType->find(name, Reflect::Value { self->mBinding });
            if (it == Reflect::ScopeIterator { Reflect::Value { self->mBinding }, nullptr }) {
                PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
                return NULL;
            }
            Reflect::Value v;
            PYTHON3_PROPAGATE_ERROR(fromPyObject(v, value));
            PYTHON3_PROPAGATE_ERROR((*it) = v);
            return 0;
        }

        /* static PyObject *TypedScopePtr_iter(const ScopePtr &p)
        {
            if (!p) {
                PyErr_SetString(PyExc_TypeError, "Nullptr is not iterable!");
                return NULL;
            }
            ScopeIterator proxyIt = p.find("__proxy");
            if (proxyIt != p.end()) {
                ValueType proxy;
                PYTHON3_PROPAGATE_ERROR(proxyIt->value(proxy));
                if (proxy.is<ScopePtr>()) {
                    return TypedScopePtr_iter(proxy.as<ScopePtr>());
                }
            }
            return toPyObject(p.begin());
        }

        static PyObject *
        PyTypedScopePtr_iter(PyTypedScopePtr *self)
        {
            return TypedScopePtr_iter(self->mPtr);
        }*/

        static PyObject *PyScopeBinding_str(PyScopeBinding *self)
        {
            PyObject *result = nullptr;
            bool success = Execution::access_binding(self->mBinding, [&](const Reflect::Value &v) {
                result = PyUnicode_FromString(v.toShortString().c_str());
            });
            if (!success) {
                PyErr_SetString(PyExc_ReferenceError, "TODO");
            }
            return result;
        }

        static PyObject *PyScopeBinding_await(PyScopeBinding *self)
        {
            PyObject *result = nullptr;
            bool success = Execution::access_binding(self->mBinding, [&](const Reflect::Value &v) {
                result = PyObject_CallFunctionObjArgs(PyObjectPtr { toPyObject(v) }.get("__await__"), NULL);
            });
            if (!success) {
                PyErr_SetString(PyExc_ReferenceError, "TODO");
            }
            return result;
        }

        PyAsyncMethods PyScopeBindingAsyncMethods = {
            .am_await = (unaryfunc)PyScopeBinding_await
        };

        PyTypeObject PyScopeBindingType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.ScopeBinding",
            .tp_basicsize = sizeof(PyScopeBinding),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyScopeBinding, &PyScopeBinding::mBinding>,
            .tp_as_async = &PyScopeBindingAsyncMethods,
            .tp_repr = (reprfunc)PyScopeBinding_str,
            .tp_str = (reprfunc)PyScopeBinding_str,
            .tp_getattro = (getattrofunc)PyScopeBinding_get,
            .tp_setattro = (setattrofunc)PyScopeBinding_set,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueScopeBinding",
            //.tp_iter = (getiterfunc)PyBinding_iter,
            .tp_new = PyType_GenericNew,
        };

    }
}
}