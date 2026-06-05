#include "../python3lib.h"

#include "pyownedscopeptr.h"

#include "Meta/reflect/scopeiterator.h"
#include "Meta/reflect/value.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *
        PyOwnedScopePtr_get(PyOwnedScopePtr *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            Reflect::ScopeIterator it = self->mPtr.get().find(name);

            if (it == self->mPtr.get().end()) {
                PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
                return NULL;
            }
            Reflect::Value v;
            PYTHON3_PROPAGATE_ERROR(it->value(v));
            return toPyObject(v);
        }

        static PyObject *OwnedScopePtr_iter(const Reflect::ScopePtr &p)
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
                    return OwnedScopePtr_iter(proxy.as<Reflect::ScopePtr>());
                }
            }
            return toPyObject(p.begin());
        }

        static PyObject *
        PyOwnedScopePtr_iter(PyOwnedScopePtr *self)
        {
            return OwnedScopePtr_iter(self->mPtr.get());
        }

        static PyObject *
        PyOwnedScopePtr_str(PyOwnedScopePtr *self)
        {
            return PyUnicode_FromString(self->mPtr.name().c_str());
        }

        PyTypeObject PyOwnedScopePtrType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.OwnedScopePtr",
            .tp_basicsize = sizeof(PyOwnedScopePtr),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyOwnedScopePtr, &PyOwnedScopePtr::mPtr>,
            .tp_str = (reprfunc)PyOwnedScopePtr_str,
            .tp_getattro = (getattrofunc)PyOwnedScopePtr_get,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of OwnedScopePtr",
            .tp_iter = (getiterfunc)PyOwnedScopePtr_iter,
            .tp_new = PyType_GenericNew,
        };

    }
}
}