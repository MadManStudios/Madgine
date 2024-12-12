#include "../python3lib.h"

#include "pyobjectutil.h"

#include "math/pymatrix3.h"
#include "math/pymatrix4.h"
#include "math/pyquaternion.h"
#include "math/pyvector2.h"
#include "math/pyvector3.h"
#include "math/pyvector4.h"
#include "pyapifunction.h"
#include "pyboundapifunction.h"
#include "pydictptr.h"
#include "pylistptr.h"
#include "pyownedscopeptr.h"
#include "pyscopeiterator.h"
#include "pytypedscopeptr.h"
#include "pyvirtualiterator.h"
#include "pyvirtualrange.h"
#include "pysender.h"
#include "pyflags.h"
#include "pyenum.h"

#include "pymoduleptr.h"

#include "Meta/keyvalue/keyvaluepair.h"
#include "Meta/keyvalue/valuetype.h"

#include "Meta/keyvalue/objectinstance.h"
#include "Meta/keyvalue/objectptr.h"
#include "pyobjectiter.h"

#include "pyobjectptr.h"

#include "python3lock.h"

#include "Madgine/behaviorreceiver.h"

#include "pyexecution.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct PyObjectInstance : ObjectInstance {

            PyObjectInstance(PyObject *obj)
                : mPtr(obj)
            {
            }

            ~PyObjectInstance() {
                Python3InnerLock lock;
                mPtr.reset();
            }

            virtual bool getValue(ValueType &retVal, std::string_view name) const override
            {
                retVal = fromPyObject(mPtr.get(name));
                return true;
            }

            virtual void setValue(std::string_view name, const ValueType &value) override
            {
                throw 0;
            }

            virtual std::map<std::string_view, ValueType> values() const override
            {
                Python3InnerLock lock;
                std::map<std::string_view, ValueType> results;
                std::ranges::transform(mPtr, std::inserter(results, results.end()), [](std::pair<PyObject *, PyObject *> p) {
                    return std::make_pair(PyUnicode_AsUTF8(p.first), fromPyObject(p.second));
                });
                return results;
            }

            virtual void call(ValueType &retVal, const ArgumentList &args) override
            {
                Python3InnerLock lock;

                retVal = fromPyObject(mPtr.call(args));
            }

            virtual std::string descriptor() const override
            {
                Python3InnerLock lock;
                PyObjectPtr repr = PyObject_Repr(mPtr);
                return PyUnicode_AsUTF8(repr);
            }

            PyObject *get() const
            {
                return mPtr;
            }

            PyObjectPtr mPtr;
        };

        PyObject *toPyObject(const ValueType &val)
        {
            return val.visit([](const auto &e) -> PyObject * {
                return toPyObject(e);
            });
        }

        PyObject *toPyObject(std::monostate)
        {
            Py_RETURN_NONE;
        }

        PyObject *toPyObject(int i)
        {
            return PyLong_FromLong(i);
        }

        PyObject *toPyObject(uint64_t i)
        {
            return PyLong_FromLong(i);
        }

        PyObject *toPyObject(bool b)
        {
            return PyBool_FromLong(b);
        }

        PyObject *toPyObject(float f)
        {
            return PyFloat_FromDouble(f);
        }

        PyObject *toPyObject(std::chrono::nanoseconds d)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <duration> yet");
            return NULL;
        }

        PyObject *toPyObject(const ScopePtr &scope)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyTypedScopePtrType, NULL);
            new (&reinterpret_cast<PyTypedScopePtr *>(obj)->mPtr) ScopePtr(scope);
            return obj;
        }

        PyObject *toPyObject(const OwnedScopePtr &scope)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyOwnedScopePtrType, NULL);
            new (&reinterpret_cast<PyOwnedScopePtr *>(obj)->mPtr) OwnedScopePtr(scope);
            return obj;
        }

        PyObject *toPyObject(const ScopeIterator &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyScopeIteratorType, NULL);
            new (&reinterpret_cast<PyScopeIterator *>(obj)->mIt) ScopeIterator(it);
            return obj;
        }

        PyObject *toPyObject(const ApiFunction &function)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyApiFunctionType, NULL);
            new (&reinterpret_cast<PyApiFunction *>(obj)->mFunction) ApiFunction(function);
            return obj;
        }

        PyObject *toPyObject(const BoundApiFunction &function)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyBoundApiFunctionType, NULL);
            new (&reinterpret_cast<PyBoundApiFunction *>(obj)->mFunction) BoundApiFunction(function);
            return obj;
        }

        PyObject *toPyObject(const KeyValueVirtualSequenceRange &range)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualSequenceRangeType, NULL);
            new (&reinterpret_cast<PyVirtualSequenceRange *>(obj)->mRange) KeyValueVirtualSequenceRange(range);
            return obj;
        }

        PyObject *toPyObject(const KeyValueVirtualAssociativeRange &range)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualAssociativeRangeType, NULL);
            new (&reinterpret_cast<PyVirtualAssociativeRange *>(obj)->mRange) KeyValueVirtualAssociativeRange(range);
            return obj;
        }

        PyObject *toPyObject(const VirtualIterator<ValueType> &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualSequenceIteratorType, NULL);
            new (&reinterpret_cast<PyVirtualSequenceIterator *>(obj)->mIt) VirtualIterator<ValueType>(it);
            return obj;
        }

        PyObject *toPyObject(const VirtualIterator<KeyValuePair> &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualAssociativeIteratorType, NULL);
            new (&reinterpret_cast<PyVirtualAssociativeIterator *>(obj)->mIt) VirtualIterator<KeyValuePair>(it);
            return obj;
        }

        PyObject *toPyObject(const CoWString &s)
        {
            return PyUnicode_FromStringAndSize(s.data(), s.size());
        }

        PyObject *toPyObject(const Vector2 &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector2Type, NULL);
            new (&reinterpret_cast<PyVector2 *>(obj)->mVector) Vector2(v);
            return obj;
        }

        PyObject *toPyObject(const Vector3 &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector3Type, NULL);
            new (&reinterpret_cast<PyVector3 *>(obj)->mVector) Vector3(v);
            return obj;
        }

        PyObject *toPyObject(const Vector4 &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector4Type, NULL);
            new (&reinterpret_cast<PyVector4 *>(obj)->mVector) Vector4(v);
            return NULL;
        }

        PyObject *toPyObject(const Vector2i &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Vector2> yet");
            return NULL;
        }

        PyObject *toPyObject(const Vector3i &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector3Type, NULL);
            new (&reinterpret_cast<PyVector3 *>(obj)->mVector) Vector3(v);
            return obj;
        }

        PyObject *toPyObject(const Vector4i &v)
        {
            /* PyObject *obj = PyObject_CallObject((PyObject *)&PyVector4Type, NULL);
            new (&reinterpret_cast<PyVector4 *>(obj)->mVector) Vector4(v);*/
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Vector4i> yet");
            return NULL;
        }

        PyObject *toPyObject(const Quaternion &q)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyQuaternionType, NULL);
            new (&reinterpret_cast<PyQuaternion *>(obj)->mQuaternion) Quaternion(q);
            return obj;
        }

        PyObject *toPyObject(const ObjectPtr &o)
        {
            if (!o)
                Py_RETURN_NONE;
            if (const PyObjectInstance *instance = dynamic_cast<const PyObjectInstance *>(o.get())) {
                PyObject *ptr = instance->get();
                Py_INCREF(ptr);
                return ptr;
            }
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <ObjectPtr> yet");
            return NULL;
        }

        PyObject *toPyObject(const CoW<Matrix3> &m)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyMatrix3Type, NULL);
            new (&reinterpret_cast<PyMatrix3 *>(obj)->mMatrix) Matrix3(m);
            return obj;
        }

        PyObject *toPyObject(const CoW<Matrix4> &m)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyMatrix4Type, NULL);
            new (&reinterpret_cast<PyMatrix4 *>(obj)->mMatrix) Matrix4(m);
            return obj;
        }

        PyObject *toPyObject(const EnumHolder &e)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyEnumType, NULL);
            new (&reinterpret_cast<PyEnum *>(obj)->mEnum) EnumHolder(e);
            return obj;
        }

        PyObject *toPyObject(const FlagsHolder &f)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyFlagsType, NULL);
            new (&reinterpret_cast<PyFlags *>(obj)->mFlags) FlagsHolder(f);
            return obj;
        }

        PyObject *toPyObject(const KeyValueFunction &f)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Function> yet");
            return nullptr;
        }

        PyObject *toPyObject(const KeyValueSender &s)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PySenderType, NULL);
            new (&reinterpret_cast<PySender *>(obj)->mSender) KeyValueSender(s);
            return obj;
        }

        PyObject *toPyObject(const ValueTypeDesc &t)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <type> yet");
            return nullptr;
        }

        struct Functor_to_KeyValuePair {
            void operator()(KeyValuePair &p, const std::pair<PyObject *, PyObject *> &o)
            {
                p.mKey = fromPyObject(o.first);
                p.mValue = fromPyObject(o.second);
            }
        };

        struct Functor_to_ValueRef {
            void operator()(ValueType &r, PyObject *o)
            {
                r = fromPyObject(o);
            }
        };

        ValueType fromPyObject(PyObject *obj)
        {
            if (!obj) {
                PyErr_Print();
                throw 0;
            } else if (obj == Py_None) {
                return ValueType { std::monostate {} };
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                return ValueType { std::string { s } };
            } else if (PyBool_Check(obj)) {
                return ValueType { obj == Py_True };
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                return ValueType { i };
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                return ValueType { KeyValueVirtualAssociativeRange { PyDictPtr { obj }, Engine::type_holder<Functor_to_KeyValuePair> } };
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                return ValueType { KeyValueVirtualSequenceRange { PyListPtr { obj }, Engine::type_holder<Functor_to_ValueRef> } };
            } else if (obj->ob_type == &PyTypedScopePtrType) {
                return ValueType { reinterpret_cast<PyTypedScopePtr *>(obj)->mPtr };
            } else {
                Py_INCREF(obj);
                return ValueType { ObjectPtr { std::make_unique<PyObjectInstance>(obj) } };
            }
        }

        void fromPyObject(BehaviorReceiver &receiver, PyObject *obj)
        {
            if (!obj) {                
                receiver.set_error(fetchError());
            } else if (obj == Py_None) {
                receiver.set_value(std::monostate {});
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                receiver.set_value(std::string { s });
            } else if (PyBool_Check(obj)) {
                receiver.set_value(obj == Py_True);
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                receiver.set_value(i);
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                receiver.set_value(KeyValueVirtualAssociativeRange { PyDictPtr { obj }, Engine::type_holder<Functor_to_KeyValuePair> });
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                receiver.set_value(KeyValueVirtualSequenceRange { PyListPtr { obj }, Engine::type_holder<Functor_to_ValueRef> });
            } else if (obj->ob_type == &PyTypedScopePtrType) {
                receiver.set_value(reinterpret_cast<PyTypedScopePtr *>(obj)->mPtr);
            } else {
                Py_INCREF(obj);
                receiver.set_value(ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
            }
        }

        ExtendedValueTypeDesc PyToValueTypeDesc(PyObject *obj)
        {
            assert(PyType_Check(obj));
            PyTypeObject *type = reinterpret_cast<PyTypeObject *>(obj);
            if (type == &PyUnicode_Type) {
                return toValueTypeDesc<std::string>();
            } else if (obj == PyModulePtr { "inspect" }.get("Parameter").get("empty")) {
                return toValueTypeDesc<ValueType>();
            }
            throw 0;
        }

    }
}
}