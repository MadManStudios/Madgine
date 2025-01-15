#pragma once

#include "Modules/threading/taskfuture.h"

#include "Generic/opaqueptr.h"

namespace Engine {

struct MADGINE_BEHAVIOR_EXPORT BehaviorHandle {

    BehaviorHandle() = default;
    BehaviorHandle(IndexType<uint32_t> index, std::string_view name);
    BehaviorHandle(const BehaviorHandle &other);
    BehaviorHandle(BehaviorHandle &&other) = default;
    ~BehaviorHandle();

    BehaviorHandle &operator=(const BehaviorHandle &other);
    BehaviorHandle &operator=(BehaviorHandle &&other);

    Behavior create(const ParameterTuple &args) const;
    Threading::TaskFuture<bool> state() const;
    Threading::TaskFuture<ParameterTuple> createParameters() const;
    ParameterTuple createDummyParameters() const;
    std::vector<ValueTypeDesc> parameterTypes() const;
    std::vector<ValueTypeDesc> resultTypes() const;
    std::vector<BindingDescriptor> bindings() const;    
    
    std::string_view name() const;

    std::string toString() const;
    bool fromString(std::string_view s);

    explicit operator bool() const;

    IndexType<uint32_t> mIndex;    
    UniqueOpaquePtr mHandle;
};
}
