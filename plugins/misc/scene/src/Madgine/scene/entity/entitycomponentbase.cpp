#include "../../scenelib.h"
#include "entitycomponentbase.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/serializetable_impl.h"

#include "entity.h"
#include "../scenemanager.h"
#include "entitycomponentlistbase.h"

METATABLE_BEGIN(Engine::Scene::Entity::EntityComponentBase)
METATABLE_END(Engine::Scene::Entity::EntityComponentBase)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::SyncableEntityComponentBase)
SERIALIZETABLE_END(Engine::Scene::Entity::SyncableEntityComponentBase)

namespace Engine {
namespace Scene {
    namespace Entity {

        SyncableEntityComponentBase::SyncableEntityComponentBase()
        {
            //This should never be thrown. Provide an Entity* to your component.
            throw 0;
        }

        SyncableEntityComponentBase::SyncableEntityComponentBase(Entity *entity)
            :mEntity(entity)
        {
        }

        bool SyncableEntityComponentBase::isMaster() const
        {
            return mEntity->isMaster();
        }

        void SyncableEntityComponentBase::writeAction(OffsetPtr offset, size_t componentIndex, void *data, Serialize::ParticipantId answerTarget, Serialize::MessageId answerId, const std::set<Serialize::ParticipantId> &targets) const
        {            
            EntityComponentActionPayload payload;
            payload.mComponentIndex = componentIndex;            
            payload.mOffset = offset;
            payload.mComponent = this;
            payload.mData = data;

            std::vector<Serialize::WriteMessage> streams = getMasterActionMessageTargets(mEntity, answerTarget, answerId, targets);
            mEntity->serializeType()->writeAction(mEntity, 1, streams, &payload);
        }

        void Engine::Scene::Entity::SyncableEntityComponentBase::writeRequest(OffsetPtr offset, void *data, Serialize::ParticipantId requester, Serialize::MessageId requesterTransactionId, Serialize::GenericMessageReceiver receiver) const
        {
            throw 0;
        }

        Serialize::WriteMessage Engine::Scene::Entity::SyncableEntityComponentBase::getSlaveRequestMessageTarget() const
        {
            throw 0;
        }

    }
}
}
