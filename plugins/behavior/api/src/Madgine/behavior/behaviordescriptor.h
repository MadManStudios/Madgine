#pragma once

#include "Generic/closure.h"

#include "Meta/reflect/accessor.h"
#include "Meta/reflect/type.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorDescriptor {

        struct ParameterStorage {
            std::string mName;
            const Type::StorageOps *mType;
        };

        struct Parameter {
            std::string_view mName;
            const Type::StorageOps **mType;
        };

        size_t parameterCount() const;
        std::string_view parameterName(size_t i) const;
        Reflect::ExtendedType parameterType(size_t i) const;
        const Type::StorageOps &parameterStorage(size_t i) const;

        size_t resultCount() const;
        Reflect::ExtendedType resultType(size_t i) const;

        size_t subBehaviorCount() const;

        std::span<const Parameter> mParameters;
        std::span<const Reflect::ExtendedType> mResultTypes;
        size_t mSubBehaviorCount;
    };

}
}