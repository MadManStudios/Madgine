#pragma once

#include "Meta/reflect/metatable.h"

#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/debug/continuation.h"

#include "python3fileloader.h"
#include "util/pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyNamedType;

        PyObject *PyEngine_Behavior_decorator(PyObject *self,
            PyObject *const *args,
            Py_ssize_t nargs);

        struct Python3BehaviorFactory : BehaviorFactory<Python3BehaviorFactory> {
            std::vector<std::string_view> names() const override;
            UniqueOpaquePtr load(std::string_view name) const override;
            Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
            void release(UniqueOpaquePtr &ptr) const override;
            std::string_view name(const UniqueOpaquePtr &handle) const override;
            Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const override;
            ParameterTuple createParameters(const UniqueOpaquePtr &handle) const override;
            std::vector<Reflect::ExtendedType> parameterTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<Reflect::ExtendedType> resultTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<NamedDescriptor> namedInputs(const UniqueOpaquePtr &handle) const override;
            size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;

            struct Entry {
                Entry(PyObjectPtr function);

                static std::unique_ptr<Reflect::Accessor[]> accessors(const PythonFunctionInfo &info);

                PyObjectPtr mFunction;
                PythonFunctionInfo mInfo;

                std::string mTupleName;
                std::unique_ptr<Reflect::Accessor[]> mTupleAccessors;
                Reflect::MetaTable mMetaTable;
                const Reflect::MetaTable *mMetaTablePtr = &mMetaTable;
            };

            std::map<std::string_view, Entry, std::less<>> mBehaviorObjects;
        };

    }
}
}