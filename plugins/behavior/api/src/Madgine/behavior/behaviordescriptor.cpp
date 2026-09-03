#include "../behaviorlib.h"

#include "behaviordescriptor.h"

#include "Meta/reflect/argumentlist.h"
#include "Meta/reflect/value.h"
#include "Meta/type/storageops.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Behavior::BehaviorDescriptor::ParameterStorage)
    MEMBER(mName)
METATABLE_END(Engine::Behavior::BehaviorDescriptor::ParameterStorage)

SERIALIZETABLE_BEGIN(Engine::Behavior::BehaviorDescriptor::ParameterStorage)
    FIELD(mName)
    FIELD(mType)
SERIALIZETABLE_END(Engine::Behavior::BehaviorDescriptor::ParameterStorage)

namespace Engine {
namespace Behavior {

    size_t BehaviorDescriptor::parameterCount() const
    {
        return mParameters.size();
    }

    std::string_view BehaviorDescriptor::parameterName(size_t i) const
    {
        return mParameters[i].mName;
    }

    Reflect::ExtendedType BehaviorDescriptor::parameterType(size_t i) const
    {
        return (*mParameters[i].mType)->mType;
    }

    const Type::StorageOps &BehaviorDescriptor::parameterStorage(size_t i) const
    {
        return **mParameters[i].mType;
    }

    size_t BehaviorDescriptor::resultCount() const
    {
        return mResultTypes.size();
    }

    Reflect::ExtendedType BehaviorDescriptor::resultType(size_t i) const
    {
        return mResultTypes[i];
    }

    size_t BehaviorDescriptor::subBehaviorCount() const
    {
        return mSubBehaviorCount;
    }

}
}