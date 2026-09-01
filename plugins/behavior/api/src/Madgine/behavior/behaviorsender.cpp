#include "../behaviorlib.h"

#include "behaviorsender.h"

#include "Meta/serialize/serializetable_impl.h"
#include "Meta/reflect/metatable_impl.h"

#include "behaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

METATABLE_BEGIN(Engine::Behavior::BehaviorSender)
    MEMBER(mParameters)
METATABLE_END(Engine::Behavior::BehaviorSender)

SERIALIZETABLE_BEGIN(Engine::Behavior::BehaviorSender)
    FIELD(mParameters)
SERIALIZETABLE_END(Engine::Behavior::BehaviorSender)

namespace Engine {
namespace Behavior {

    BehaviorSender::BehaviorSender(BehaviorHandle handle)
        : mHandle(std::move(handle))
        , mParameters(BehaviorFactoryRegistry::get(mHandle.mIndex).mFactory->createParameters(mHandle.mHandle))
    {
    }

    Behavior BehaviorSender::create() const
    {
        return mHandle.create(mParameters);
    }

}
}
