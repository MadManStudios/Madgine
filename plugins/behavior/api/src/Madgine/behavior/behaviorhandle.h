#pragma once

#include "Generic/opaqueptr.h"

#include "Modules/threading/taskfuture.h"

#include "behavior.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorHandle {

        BehaviorHandle() = default;
        BehaviorHandle(IndexType<uint32_t> index, std::string_view name);
        BehaviorHandle(const BehaviorHandle &other);
        BehaviorHandle(BehaviorHandle &&other) = default;
        ~BehaviorHandle();

        BehaviorHandle &operator=(const BehaviorHandle &other);
        BehaviorHandle &operator=(BehaviorHandle &&other);

        void reset();

        Behavior create(const ParameterTuple &args, std::vector<Behavior> behaviors = {}) const;
        Threading::TaskFuture<bool> state() const;
        ParameterTuple createParameters() const;
        std::vector<Reflect::ExtendedType> parameterTypes() const;
        std::vector<Reflect::ExtendedType> resultTypes() const;
        std::vector<NamedDescriptor> namedInputs() const;
        size_t subBehaviorCount() const;

        std::string_view name() const;

        std::string toString() const;
        bool fromString(std::string_view s);

        explicit operator bool() const;

        IndexType<uint32_t> mIndex;
        UniqueOpaquePtr mHandle;
    };
}
}
