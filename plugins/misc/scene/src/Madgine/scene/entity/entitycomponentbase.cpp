#include "../../scenelib.h"
#include "entitycomponentbase.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/serializetable_impl.h"

#include "../scenemanager.h"
#include "entity.h"
#include "entitycomponentlistbase.h"

METATABLE_BEGIN(Engine::Scene::Entity::EntityComponentBase)
METATABLE_END(Engine::Scene::Entity::EntityComponentBase)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::SyncableEntityComponentBase)
SERIALIZETABLE_END(Engine::Scene::Entity::SyncableEntityComponentBase)

namespace Engine {
namespace Scene {
    namespace Entity {

        EntityComponentBase::EntityComponentBase(Entity *entity)
            : mEntity(entity)
        {
        }

        Entity *EntityComponentBase::entity() const
        {
            return mEntity;
        }

        bool SyncableEntityComponentBase::isMaster() const
        {
            return entity()->isMaster();
        }

        void SyncableEntityComponentBase::writeAction(OffsetPtr offset, size_t componentIndex, void *data, Serialize::ParticipantId answerTarget, Serialize::MessageId answerId, const std::set<Serialize::ParticipantId> &targets) const
        {
            EntityComponentActionPayload payload;
            payload.mComponentIndex = componentIndex;
            payload.mOffset = offset;
            payload.mComponent = this;
            payload.mData = data;

            std::vector<Serialize::WriteMessage> streams = getMasterActionMessageTargets(entity(), answerTarget, answerId, targets);
            entity()->serializeType()->writeAction(entity(), 1, streams, &payload);
        }

        void Engine::Scene::Entity::SyncableEntityComponentBase::writeRequest(OffsetPtr offset, void *data, Serialize::ParticipantId requester, Serialize::MessageId requesterTransactionId, Serialize::GenericMessageReceiver receiver) const
        {
            throw 0;
        }

        Serialize::WriteMessage getSlaveRequestMessageTarget(const EntityComponentBase *unit, Serialize::ParticipantId requester, Serialize::MessageId requestId, Serialize::GenericMessageReceiver receiver)
        {
            throw 0;
        }

    }
}
}
