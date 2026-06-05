#include "../python3lib.h"

#include "pyscopeptr.h"

#include "Meta/reflect/scopeiterator.h"
#include "Meta/reflect/value.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *PyTypedScopePtr_get(PyTypedScopePtr *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            Reflect::ScopeIterator it = self->mPtr.find(name);

            if (it == self->mPtr.end()) {
                PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
                return NULL;
            }
            Reflect::Value v;
            PYTHON3_PROPAGATE_ERROR(it->value(v));
            return toPyObject(v);
        }

        static PyObject *TypedScopePtr_iter(const Reflect::ScopePtr &p)
        {
            if (!p) {
                PyErr_SetString(PyExc_TypeError, "Nullptr is not iterable!");
                return NULL;
            }
            Reflect::ScopeIterator proxyIt = p.find("__proxy");
            if (proxyIt != p.end()) {
                Reflect::Value proxy;
                PYTHON3_PROPAGATE_ERROR(proxyIt->value(proxy));
                if (proxy.is<Reflect::ScopePtr>()) {
                    return TypedScopePtr_iter(proxy.as<Reflect::ScopePtr>());
                }
            }
            return toPyObject(p.begin());
        }

        static PyObject *PyTypedScopePtr_iter(PyTypedScopePtr *self)
        {
            return TypedScopePtr_iter(self->mPtr);
        }

        static PyObject *PyTypedScopePtr_str(PyTypedScopePtr *self)
        {
            return PyUnicode_FromString(self->mPtr.name().c_str());
        }

        PyTypeObject PyTypedScopePtrType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.ScopePtr",
            .tp_basicsize = sizeof(PyTypedScopePtr),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyTypedScopePtr, &PyTypedScopePtr::mPtr>,
            .tp_repr = (reprfunc)PyTypedScopePtr_str,
            .tp_str = (reprfunc)PyTypedScopePtr_str,
            .tp_getattro = (getattrofunc)PyTypedScopePtr_get,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of ScopePtr",
            .tp_iter = (getiterfunc)PyTypedScopePtr_iter,
            .tp_new = PyType_GenericNew,
        };

    }
}
}