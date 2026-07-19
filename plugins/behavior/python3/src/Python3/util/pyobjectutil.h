#pragma once

#include "Meta/reflect/result.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        template <typename T, typename V>
        void PyDealloc(V(T::*p), PyObject *self)
        {
            ((T *)(self)->*p).~V();
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
            ss << (((T *)self)->*p);
            return PyUnicode_FromString(ss.str().c_str());
        }

        struct VirtualRangeHelper {
            void operator()(CallableView<void(const Reflect::Value &)> cb, PyObject *o);
            void operator()(CallableView<void(const Reflect::Value &, const Reflect::Value &)> cb, const std::pair<PyObject *, PyObject *> &o);
        };

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Value &val);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(std::monostate);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(int i);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(uint64_t i);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(bool b);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(float f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(Reflect::Duration64 d);

        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::ScopePtr &scope);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::OwnedValue &value);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::ScopeIterator &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::ApiFunction &function);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::BoundApiFunction &function);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::SequenceRange &range);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::AssociativeRange &range);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::AssociativeIterator &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::SequenceIterator &it);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoWString &s);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector4 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector3 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector2 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector4i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector3i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Vector2i &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Color3 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Color4 &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Math::Quaternion &v);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::ObjectPtr &o);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoW<Math::Matrix3> &m);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const CoW<Math::Matrix4> &m);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Enum &e);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Flags &f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Function &f);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Sender &s);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::Binding &b);
        MADGINE_PYTHON3_EXPORT PyObject *toPyObject(const Reflect::ScopeBinding &b);

        MADGINE_PYTHON3_EXPORT Reflect::Result fromPyObject(Reflect::Value &result, PyObject *obj);

        MADGINE_PYTHON3_EXPORT PyObject *toPyError(const Reflect::Error &);
        MADGINE_PYTHON3_EXPORT Reflect::Error fromPyError(PyObject *exc, PyObject *traceback = nullptr);

        MADGINE_PYTHON3_EXPORT Reflect::Error fetchError();

        MADGINE_PYTHON3_EXPORT Reflect::ExtendedType PyToValueTypeDesc(PyObject *obj);
        MADGINE_PYTHON3_EXPORT const Type::StorageOps *PyToStorageOps(PyObject *obj);

        MADGINE_PYTHON3_EXPORT PyObject *toPyTuple(const Reflect::ArgumentList &args);

    }
}
}

#define PYTHON3_PROPAGATE_ERROR(...)                       \
    if (::Engine::Reflect::Result _result = (__VA_ARGS__)) \
    return toPyError(*_result.mError)