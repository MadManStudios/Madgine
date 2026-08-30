#include "../python3lib.h"

#include "pyobjectutil.h"

#include "Meta/reflect/objectinstance.h"
#include "Meta/reflect/objectptr.h"
#include "Meta/reflect/value.h"
#include "Meta/type/storageops.h"

#include "Madgine/behavior/behaviorreceiver.h"

#include "../python3behaviors.h"
#include "../python3env.h"
#include "math/pymatrix3.h"
#include "math/pymatrix4.h"
#include "math/pyquaternion.h"
#include "math/pyvector2.h"
#include "math/pyvector3.h"
#include "math/pyvector4.h"
#include "pyapifunction.h"
#include "pybinding.h"
#include "pyboundapifunction.h"
#include "pydictptr.h"
#include "pyduration.h"
#include "pyenum.h"
#include "pyflags.h"
#include "pylistptr.h"
#include "pymoduleptr.h"
#include "pyobjectinstance.h"
#include "pyobjectiter.h"
#include "pyobjectptr.h"
#include "pyownedvalue.h"
#include "pyscopeiterator.h"
#include "pyscopeptr.h"
#include "pysender.h"
#include "python3lock.h"
#include "pytype.h"
#include "pyvirtualiterator.h"
#include "pyvirtualrange.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *toPyObject(const Reflect::Value &val)
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

        PyObject *toPyObject(Reflect::Duration64 d)
        {
            PyObject *obj = PyObject_CallFunction((PyObject *)&PyDurationType, "K", d.count());
            // new (&reinterpret_cast<PyDuration *>(obj)->mDuration) Reflect::Duration64(d);
            return obj;
        }

        PyObject *toPyObject(const Reflect::ScopePtr &scope)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyScopePtrType, NULL);
            new (&reinterpret_cast<PyScopePtr *>(obj)->mPtr) Reflect::ScopePtr(scope);
            return obj;
        }

        PyObject *toPyObject(const Reflect::OwnedValue &value)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyOwnedValueType, NULL);
            new (&reinterpret_cast<PyOwnedValue *>(obj)->mValue) Reflect::OwnedValue(value);
            return obj;
        }

        PyObject *toPyObject(const Reflect::ScopeIterator &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyScopeIteratorType, NULL);
            new (&reinterpret_cast<PyScopeIterator *>(obj)->mIt) Reflect::ScopeIterator(it);
            return obj;
        }

        PyObject *toPyObject(const Reflect::ApiFunction &function)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyApiFunctionType, NULL);
            new (&reinterpret_cast<PyApiFunction *>(obj)->mFunction) Reflect::ApiFunction(function);
            return obj;
        }

        PyObject *toPyObject(const Reflect::BoundApiFunction &function)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyBoundApiFunctionType, NULL);
            new (&reinterpret_cast<PyBoundApiFunction *>(obj)->mFunction) Reflect::BoundApiFunction(function);
            return obj;
        }

        PyObject *toPyObject(const Reflect::SequenceRange &range)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualSequenceRangeType, NULL);
            new (&reinterpret_cast<PyVirtualSequenceRange *>(obj)->mRange) Reflect::SequenceRange(range);
            return obj;
        }

        PyObject *toPyObject(const Reflect::AssociativeRange &range)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualAssociativeRangeType, NULL);
            new (&reinterpret_cast<PyVirtualAssociativeRange *>(obj)->mRange) Reflect::AssociativeRange(range);
            return obj;
        }

        PyObject *toPyObject(const Reflect::SequenceIterator &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualSequenceIteratorType, NULL);
            new (&reinterpret_cast<PyVirtualSequenceIterator *>(obj)->mIt) Reflect::SequenceIterator(it);
            return obj;
        }

        PyObject *toPyObject(const Reflect::AssociativeIterator &it)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVirtualAssociativeIteratorType, NULL);
            new (&reinterpret_cast<PyVirtualAssociativeIterator *>(obj)->mIt) Reflect::AssociativeIterator(it);
            return obj;
        }

        PyObject *toPyObject(const CoWString &s)
        {
            return PyUnicode_FromStringAndSize(s.data(), s.size());
        }

        PyObject *toPyObject(const Math::Vector2 &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector2Type, NULL);
            new (&reinterpret_cast<PyVector2 *>(obj)->mVector) Math::Vector2(v);
            return obj;
        }

        PyObject *toPyObject(const Math::Vector3 &v)
        {
            PyObject *obj = PyObject_CallFunction((PyObject *)&PyVector3Type, "fff", v.x, v.y, v.z);
            // new (&reinterpret_cast<PyVector3 *>(obj)->mVector) Math::Vector3(v);
            return obj;
        }

        PyObject *toPyObject(const Math::Vector4 &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector4Type, NULL);
            new (&reinterpret_cast<PyVector4 *>(obj)->mVector) Math::Vector4(v);
            return NULL;
        }

        PyObject *toPyObject(const Math::Vector2i &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Vector2> yet");
            return NULL;
        }

        PyObject *toPyObject(const Math::Color3 &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Color3> yet");
            return NULL;
        }

        PyObject *toPyObject(const Math::Color4 &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Color4> yet");
            return NULL;
        }

        PyObject *toPyObject(const Math::Vector3i &v)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyVector3Type, NULL);
            new (&reinterpret_cast<PyVector3 *>(obj)->mVector) Math::Vector3(v);
            return obj;
        }

        PyObject *toPyObject(const Math::Vector4i &v)
        {
            /* PyObject *obj = PyObject_CallObject((PyObject *)&PyVector4Type, NULL);
            new (&reinterpret_cast<PyVector4 *>(obj)->mVector) Vector4(v);*/
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Vector4i> yet");
            return NULL;
        }

        PyObject *toPyObject(const Math::Quaternion &q)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyQuaternionType, NULL);
            new (&reinterpret_cast<PyQuaternion *>(obj)->mQuaternion) Math::Quaternion(q);
            return obj;
        }

        PyObject *toPyObject(const Reflect::ObjectPtr &o)
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

        PyObject *toPyObject(const CoW<Math::Matrix3> &m)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyMatrix3Type, NULL);
            new (&reinterpret_cast<PyMatrix3 *>(obj)->mMatrix) Math::Matrix3(m);
            return obj;
        }

        PyObject *toPyObject(const CoW<Math::Matrix4> &m)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyMatrix4Type, NULL);
            new (&reinterpret_cast<PyMatrix4 *>(obj)->mMatrix) Math::Matrix4(m);
            return obj;
        }

        PyObject *toPyObject(const Reflect::Enum &e)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyEnumType, NULL);
            new (&reinterpret_cast<PyEnum *>(obj)->mEnum) Reflect::Enum(e);
            return obj;
        }

        PyObject *toPyObject(const Reflect::Flags &f)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyFlagsType, NULL);
            new (&reinterpret_cast<PyFlags *>(obj)->mFlags) Reflect::Flags(f);
            return obj;
        }

        PyObject *toPyObject(const Reflect::Function &f)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Function> yet");
            return nullptr;
        }

        PyObject *toPyObject(const Reflect::Sender &s)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PySenderType, NULL);
            new (&reinterpret_cast<PySender *>(obj)->mSender) Reflect::Sender(s);
            return obj;
        }

        PyObject *toPyObject(const Reflect::Binding &b)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyBindingType, NULL);
            new (&reinterpret_cast<PyBinding *>(obj)->mBinding) Reflect::Binding(b);
            return obj;
        }

        PyObject *toPyObject(const Reflect::ScopeBinding &b)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyScopeBindingType, NULL);
            new (&reinterpret_cast<PyScopeBinding *>(obj)->mBinding) Reflect::ScopeBinding(b);
            return obj;
        }

        void VirtualRangeHelper::operator()(CallableView<void(const Reflect::Value &)> cb, PyObject *o)
        {
            Reflect::Value_erased([&](Reflect::Value &v) {
                fromPyObject(v, o);
                cb(v);
            });
        }

        void VirtualRangeHelper::operator()(CallableView<void(const Reflect::Value &, const Reflect::Value &)> cb, const std::pair<PyObject *, PyObject *> &o)
        {
            Reflect::Value_erased([&](Reflect::Value &key) {
                Reflect::Value_erased([&](Reflect::Value &value) {
                    Reflect::Result result = fromPyObject(key, o.first);
                    if (result)
                        throw 0;
                    result = fromPyObject(value, o.second);
                    if (result)
                        throw 0;
                    cb(key, value);
                });
            });
        }

        Reflect::Result fromPyObject(Reflect::Value &result, PyObject *obj)
        {
            if (!obj) {
                return std::make_unique<Reflect::Error>(fetchError());
            } else if (obj == Py_None) {
                toValue(result, std::monostate {});
                return {};
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                toValue(result, std::string { s });
                return {};
            } else if (PyBool_Check(obj)) {
                toValue(result, obj == Py_True);
                return {};
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                toValue(result, i);
                return {};
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                toValue(result, Reflect::AssociativeRange { PyDictPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
                return {};
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                toValue(result, Reflect::SequenceRange { PyListPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
                return {};
            } else if (obj->ob_type == &PyScopePtrType) {
                toValue(result, reinterpret_cast<PyScopePtr *>(obj)->mPtr);
                return {};
            } else if (obj->ob_type == &PyOwnedValueType) {
                toValue(result, reinterpret_cast<PyOwnedValue *>(obj)->mValue);
                return {};
            } else if (obj->ob_type == &PyBindingType) {
                toValue(result, reinterpret_cast<PyBinding *>(obj)->mBinding);
                return {};
            } else if (obj->ob_type == &PySenderType) {
                toValue(result, reinterpret_cast<PySender *>(obj)->mSender);
                return {};
            } else if (obj->ob_type == &PyVector3Type) {
                toValue(result, reinterpret_cast<PyVector3 *>(obj)->mVector);
                return {};
            } else if (obj->ob_type == &PyQuaternionType) {
                toValue(result, reinterpret_cast<PyQuaternion *>(obj)->mQuaternion);
                return {};
            } else if (obj->ob_type == &PyDurationType) {
                toValue(result, reinterpret_cast<PyDuration *>(obj)->mDuration);
                return {};
            } else {
                Py_INCREF(obj);
                toValue(result, Reflect::ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
                return {};
            }
        }

        PyErrorToken toPyError(const Reflect::Error &err)
        {
            PyErr_SetString(PyExc_Exception, err.mMsg.c_str());

            return {};
        }

        Reflect::Error fetchError()
        {
            PyObjectPtr type, value, traceback;
            PyErr_Fetch(&type, &value, &traceback);

            return fromPyError(value, traceback);
        }

        Reflect::Error fromPyError(PyObject *exc, PyObject *traceback)
        {
            if (!traceback) {
                traceback = PyObject_GetAttrString(exc, "__traceback__");
            }

            std::vector<Reflect::Error::StackEntry> stack;

            if (traceback && !Py_IsNone(traceback)) {
                PyTracebackObject *tb = reinterpret_cast<PyTracebackObject *>(traceback);
                while (tb) {
                    stack.emplace_back(
                        PyUnicode_AsUTF8(PyFrame_GetCode(tb->tb_frame)->co_name),
                        PyUnicode_AsUTF8(PyFrame_GetCode(tb->tb_frame)->co_filename),
                        PyFrame_GetCode(tb->tb_frame)->co_firstlineno);
                    tb = tb->tb_next;
                }
            }

            PyObjectPtr str = PyObject_Str(exc);
            const char *errorMessage = PyUnicode_AsUTF8(str);

            std::string msg;
            if (errorMessage)
                msg = errorMessage;

            return { GenericResult { GenericResult::UNKNOWN_ERROR }, msg, std::move(stack) };
        }

        Reflect::ExtendedType PyToValueTypeDesc(PyObject *obj)
        {
            if (Py_IS_TYPE(obj, &PyTypeType)) {
                return { { Reflect::TypeEnum::ScopeValue }, reinterpret_cast<PyType *>(obj)->mType->mMetaTable->mSelf };
            } else if (PyType_Check(obj)) {
                PyTypeObject *type = reinterpret_cast<PyTypeObject *>(obj);
                if (type == &PyUnicode_Type) {
                    return Reflect::toType<std::string>();
                } else if (obj == PyModulePtr { "inspect" }.get("Parameter").get("empty")) {
                    return Reflect::toType<Reflect::Value>();
                }
            } else if (PyModule_Check(obj)) {
                return { { Reflect::TypeEnum::ScopeValue }, Type::resolveTypeName(PyModule_GetName(obj), ".")->mMetaTable->mSelf };
            }
            throw 0;
        }

        const Type::StorageOps *PyToStorageOps(PyObject *obj)
        {
            if (Py_IS_TYPE(obj, &PyTypeType)) {
                return *reinterpret_cast<PyType *>(obj)->mType->mMetaTable->mStorage;
            } else if (PyType_Check(obj)) {
                PyTypeObject *type = reinterpret_cast<PyTypeObject *>(obj);
                if (type == &PyUnicode_Type) {
                    return storageOps<std::string>;
                }
            }
            return nullptr;
        }

        PyObject *toPyTuple(const Reflect::ArgumentList &args)
        {
            PyObject *tuple = PyTuple_New(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                PyObject *element = toPyObject(args[i]);
                if (!element) {
                    Py_DecRef(tuple);
                    return nullptr;
                }
                PyTuple_SetItem(tuple, i, element);
            }
            return tuple;
        }

        PyObject *toPyException(const Reflect::Error &err)
        {
            PyObjectPtr msg = PyUnicode_FromString(err.mMsg.c_str());
            if (!msg)
                return nullptr;

            return PyObject_CallOneArg(PyExc_Exception, msg);
        }

    }
}
}