#include "../python3lib.h"

#include "pyobjectutil.h"

#include "Meta/keyvalue/keyvaluepair.h"
#include "Meta/keyvalue/objectinstance.h"
#include "Meta/keyvalue/objectptr.h"
#include "Meta/keyvalue/valuetype.h"

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
#include "pyenum.h"
#include "pyflags.h"
#include "pylistptr.h"
#include "pymoduleptr.h"
#include "pyobjectiter.h"
#include "pyobjectptr.h"
#include "pyownedscopeptr.h"
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

        struct PyObjectInstance : ObjectInstance {

            PyObjectInstance(PyObject *obj)
                : mPtr(obj)
            {
            }

            ~PyObjectInstance()
            {
                Python3InnerLock lock;
                mPtr.reset();
            }

            virtual KeyValueResult getValue(ValueType &retVal, std::string_view name) const override
            {
                return fromPyObject(retVal, mPtr.get(name));
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
                    ValueType v;
                    fromPyObject(v, p.second);
                    return std::make_pair(PyUnicode_AsUTF8(p.first), std::move(v));
                });
                return results;
            }

            virtual KeyValueResult call(ValueType &retVal, const ArgumentList &args) override
            {
                Python3InnerLock lock;

                return fromPyObject(retVal, mPtr.call(args));
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

        PyObject *toPyObject(const Color3 &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Color3> yet");
            return NULL;
        }

        PyObject *toPyObject(const Color4 &v)
        {
            PyErr_SetString(PyExc_NotImplementedError, "Can't convert type <Color4> yet");
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

        PyObject *toPyObject(const KeyValueBinding &b)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyBindingType, NULL);
            new (&reinterpret_cast<PyBinding *>(obj)->mBinding) KeyValueBinding(b);
            return obj;
        }

        PyObject *toPyObject(const KeyValueScopeBinding &b)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyScopeBindingType, NULL);
            new (&reinterpret_cast<PyScopeBinding *>(obj)->mBinding) KeyValueScopeBinding(b);
            return obj;
        }

        struct Functor_to_KeyValuePair {
            void operator()(KeyValuePair &p, const std::pair<PyObject *, PyObject *> &o)
            {
                KeyValueResult result = fromPyObject(p.mKey, o.first);
                if (result)
                    throw 0;
                result = fromPyObject(p.mValue, o.second);
                if (result)
                    throw 0;
            }
        };

        struct Functor_to_ValueRef {
            void operator()(ValueType &r, PyObject *o)
            {
                KeyValueResult result = fromPyObject(r, o);
                if (result)
                    throw 0;
            }
        };

        KeyValueResult fromPyObject(ValueType &result, PyObject *obj)
        {
            if (!obj) {
                return std::make_unique<KeyValueError>(fetchError());
            } else if (obj == Py_None) {
                to_ValueType(result, std::monostate {});
                return {};
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                to_ValueType(result, std::string { s });
                return {};
            } else if (PyBool_Check(obj)) {
                to_ValueType(result, obj == Py_True);
                return {};
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                to_ValueType(result, i);
                return {};
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                to_ValueType(result, KeyValueVirtualAssociativeRange { PyDictPtr { obj }, Engine::type_holder<Functor_to_KeyValuePair> });
                return {};
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                to_ValueType(result, KeyValueVirtualSequenceRange { PyListPtr { obj }, Engine::type_holder<Functor_to_ValueRef> });
                return {};
            } else if (obj->ob_type == &PyTypedScopePtrType) {
                to_ValueType(result, reinterpret_cast<PyTypedScopePtr *>(obj)->mPtr);
                return {};
            } else if (obj->ob_type == &PyBindingType) {
                to_ValueType(result, reinterpret_cast<PyBinding *>(obj)->mBinding);
                return {};
            } else if (obj->ob_type == &PySenderType){
                to_ValueType(result, reinterpret_cast<PySender *>(obj)->mSender);
                return {};
            } else {
                Py_INCREF(obj);
                to_ValueType(result, ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
                return {};
            }
        }

        void handlePyObject(PyObject *obj)
        {
            if (!obj) {
                if (PyErr_ExceptionMatches(PyExc_EOFError)) {
                    BehaviorReceiver *current = Python3Environment::unlock();
                    current->set_done();
                    Python3Environment::lock();
                } else {
                    handleKeyValueError(fetchError());
                }
                return;
            }

            BehaviorReceiver *current = Python3Environment::unlock();

            if (obj == Py_None) {
                current->set_value();
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                current->set_value(std::string { s });
            } else if (PyBool_Check(obj)) {
                current->set_value(obj == Py_True);
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                current->set_value(i);
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                current->set_value(KeyValueVirtualAssociativeRange { PyDictPtr { obj }, Engine::type_holder<Functor_to_KeyValuePair> });
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                current->set_value(KeyValueVirtualSequenceRange { PyListPtr { obj }, Engine::type_holder<Functor_to_ValueRef> });
            } else if (obj->ob_type == &PyTypedScopePtrType) {
                current->set_value(reinterpret_cast<PyTypedScopePtr *>(obj)->mPtr);
            } else if (PyTuple_Check(obj)) {
                size_t size = PyTuple_Size(obj);
                ArgumentList args { std::true_type {}, size };
                for (size_t i = 0; i < args.size(); ++i) {
                    KeyValueResult result = fromPyObject(args[i], PyTuple_GetItem(obj, i));
                    if (result) {
                        current->set_error(std::move(*result.mError));
                        return;
                    }
                }
                current->set_value(std::move(args));
            } else {
                Py_INCREF(obj);
                current->set_value(ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
            }

            Python3Environment::lock();
        }

        void handleKeyValueError(KeyValueError error)
        {
            BehaviorReceiver *current = Python3Environment::unlock();
            current->set_error(std::move(error));
            Python3Environment::lock();
        }

        extern PyTypeObject PyDebugLineType;

        void resumeCoroutine(PyObject *coroutine, PyObject *value)
        {
            PyObjectPtr coro = PyObjectPtr::fromBorrowed(coroutine);
            PyObjectPtr result;
            if (!value) {
                PyObjectPtr exc = PyObject_CallFunction(PyExc_EOFError, NULL, NULL);
                result = PyObject_CallFunctionObjArgs(coro.get("throw"), static_cast<PyObject *>(exc), NULL);
            } else {
                result = PyObject_Call(coro.get("send"), value, NULL);
            }
            if (!result) {
                if (PyErr_ExceptionMatches(PyExc_StopIteration)) {
                    PyObjectPtr type, value, traceback;
                    PyErr_Fetch(&type, &value, &traceback);

                    result = value.get("value");
                }
                handlePyObject(result);
            } else if (Py_IS_TYPE(result, &PySenderStateType)) {
                SenderState &state = reinterpret_cast<PySenderState *>(static_cast<PyObject *>(result))->mState;
                state.mCoroutine = std::move(coroutine);

                state.resume();
            } else if (Py_IS_TYPE(result, &PyDebugLineType)) {
                DebugLine &debugLine = reinterpret_cast<PyDebugLine *>(static_cast<PyObject *>(result))->mLine;
                // yield(location, rec, std::forward<F>(callback), outContinuation, type, std::forward<Args>(args)...);
                PyObject *frame = coro.get("cr_frame");
                BehaviorReceiver *current = Python3Environment::unlock();
                if (Execution::get_stop_token(*current)->registerCallback(&debugLine)) {
                    Debug::get_debug_context(*current).suspend(frame, { [coro { std::move(coro) }, current](Debug::ContinuationMode mode) mutable {
                                                                           Python3Lock lock { current };
                                                                           switch (mode) {
                                                                           case Debug::ContinuationMode::Continue:
                                                                               resumeCoroutine(coro, toPyTuple(ArgumentList { std::monostate {} }));
                                                                               break;
                                                                           case Debug::ContinuationMode::Abort:
                                                                               resumeCoroutine(coro, nullptr);
                                                                               break;
                                                                           }
                                                                           coro.reset();
                                                                       },
                                                                          Debug::ContinuationType::Flow },
                        debugLine.mContinuation, Execution::get_stop_token(*current));
                } else {
                    resumeCoroutine(coro, nullptr);
                }
                Python3Environment::lock();
            } else {
                std::string typeName = PyUnicode_AsUTF8(PyType_GetName(Py_TYPE(result)));
                BehaviorReceiver *current = Python3Environment::unlock();
                current->set_error(KEYVALUE_UNKNOWN_ERROR() << "Unknown result type from coroutine: " << typeName);
                Python3Environment::lock();
            }
        }

        PyObject *toPyError(const KeyValueError &err)
        {
            PyErr_SetString(PyExc_Exception, err.mMsg.c_str());

            return nullptr;
        }

        KeyValueError fetchError()
        {
            PyObjectPtr type, value, traceback;
            PyErr_Fetch(&type, &value, &traceback);

            return fromPyError(value, traceback);
        }

        KeyValueError fromPyError(PyObject *exc, PyObject *traceback)
        {
            const char *function = "";
            const char *filename = "";
            size_t line = 0;

            if (!traceback) {
                traceback = PyObject_GetAttrString(exc, "__traceback__");
            }

            if (traceback && !Py_IsNone(traceback)) {
                PyTracebackObject *tb = reinterpret_cast<PyTracebackObject *>(traceback);
                while (tb->tb_next)
                    tb = tb->tb_next;

                function = PyUnicode_AsUTF8(PyFrame_GetCode(tb->tb_frame)->co_name);
                filename = PyUnicode_AsUTF8(PyFrame_GetCode(tb->tb_frame)->co_filename);
                line = PyFrame_GetCode(tb->tb_frame)->co_firstlineno;
            }

            PyObjectPtr str = PyObject_Str(exc);
            const char *errorMessage = PyUnicode_AsUTF8(str);

            std::string msg;
            if (errorMessage)
                msg = errorMessage;

            return { GenericResult { GenericResult::UNKNOWN_ERROR }, msg, function, filename, line };
        }

        ExtendedValueTypeDesc PyToValueTypeDesc(PyObject *obj)
        {
            if (Py_IS_TYPE(obj, &PyTypeType)) {
                return { { ValueTypeEnum::ScopeValue }, reinterpret_cast<PyType *>(obj)->mType->mSelf };
            } else if (PyType_Check(obj)) {
                PyTypeObject *type = reinterpret_cast<PyTypeObject *>(obj);
                if (type == &PyUnicode_Type) {
                    return toValueTypeDesc<std::string>();
                } else if (obj == PyModulePtr { "inspect" }.get("Parameter").get("empty")) {
                    return toValueTypeDesc<ValueType>();
                }
            }
            throw 0;
        }

        PyObject *toPyTuple(const ArgumentList &args)
        {
            PyObject *tuple = PyTuple_New(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                PyTuple_SetItem(tuple, i, toPyObject(args[i]));
            }
            return tuple;
        }

    }
}
}