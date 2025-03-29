#pragma once

namespace Engine {
namespace Scripting {
    namespace Python3 {

        template <typename T, typename V>
        void PyDealloc(V(T::*p), PyObject *self)
        {
            ((T*)(self)->*p).~V();
        }

        template <typename T, auto p>
        void PyDealloc(PyObject *self)
        // destruct the object
        {
            PyDealloc(p, self);
            Py_TYPE(self)->tp_free(self);
        }

        template <typename T, auto p>
        static PyObject *
        PyStr(PyObject *self)
        {
            std::ostringstream ss;
            ss << (((T*)self)->*p);
            return PyUnicode_FromString(ss.str().c_str());
        }

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ValueType &val);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(std::monostate);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(int i);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(uint64_t i);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(bool b);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(float f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(std::chrono::nanoseconds d);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ScopePtr &scope);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const OwnedScopePtr &scope);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ScopeIterator &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ApiFunction &function);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const BoundApiFunction &function);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const KeyValueVirtualSequenceRange &range);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const KeyValueVirtualAssociativeRange &range);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const VirtualIterator<KeyValuePair> &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const VirtualIterator<ValueType> &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoWString &s);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector4 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector3 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector2 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector4i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector3i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Vector2i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Quaternion &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ObjectPtr &o);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoW<Matrix3> &m);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoW<Matrix4> &m);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const EnumHolder &e);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const FlagsHolder &f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const KeyValueFunction &f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const KeyValueSender &s);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const ValueTypeDesc &t);

        MADGINE_PYTHON3_EXPORT ValueType fromPyObject(PyObject *obj);
        MADGINE_PYTHON3_EXPORT void fromPyObject(BehaviorReceiver &receiver, PyObject *obj);

        MADGINE_PYTHON3_EXPORT ExtendedValueTypeDesc PyToValueTypeDesc(PyObject *obj);

    }
}
}