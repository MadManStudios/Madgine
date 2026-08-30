#pragma once

#include "Meta/reflect/metatable.h"

#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/behavior/behaviordescriptor.h"
#include "Madgine/debug/continuation.h"

#include "python3fileloader.h"
#include "util/pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyContextualType;

        PyObject *PyEngine_Behavior_decorator(PyObject *self,
            PyObject *const *args,
            Py_ssize_t nargs);

        struct Python3BehaviorFactory : BehaviorFactory<Python3BehaviorFactory> {
            std::vector<std::string_view> names() const override;
            UniqueOpaquePtr load(std::string_view name) const override;
            Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
            void release(UniqueOpaquePtr &ptr) const override;
            std::string_view name(const UniqueOpaquePtr &handle) const override;
            Behavior create(const UniqueOpaquePtr &handle, const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const override;
            ParameterTuple createParameters(const UniqueOpaquePtr &handle) const override;
            const BehaviorDescriptor &descriptor(const UniqueOpaquePtr &handle) const override;

            struct Entry {
                Entry(const char *name, PyObjectPtr function, Reflect::ExtendedType type, std::list<std::string> nameCache, std::vector<BehaviorDescriptor::Parameter> parameters);

                std::list<std::string> mNameCache;
                std::vector<BehaviorDescriptor::Parameter> mParameters;
                Reflect::ExtendedType mReturnType;

                PyObjectPtr mFunction;
                BehaviorDescriptor mDescriptor;                

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