#pragma once

#include "Meta/serialize/hierarchy/serializableunit.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "entitycomponentcontainer.h"

#include "Generic/container/compactcontainer.h"

#include "Meta/serialize/streams/pendingrequest.h"

#include "Generic/offsetptr.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityComponentActionPayload {            
            size_t mComponentIndex;
            OffsetPtr mOffset;
            const void *mComponent;
            void *mData;
        };

        struct MADGINE_SCENE_EXPORT EntityComponentBase {
            using Container = CompactContainer<std::vector<Placeholder<0>>, EntityComponentRelocateFunctor>;

            EntityComponentBase(Entity *entity);

            Entity *entity() const;

        private:
            NulledPtr<Entity> mEntity;
        };

        struct MADGINE_SCENE_EXPORT SyncableEntityComponentBase : EntityComponentBase, Serialize::SerializableUnitBase {
            using Container = CompactContainer<std::vector<Placeholder<0>>, EntityComponentRelocateFunctor>;

            using EntityComponentBase::EntityComponentBase;

            bool isMaster() const;

        protected:
            void writeAction(OffsetPtr offset, size_t componentIndex, void *data, Serialize::ParticipantId answerTarget, Serialize::MessageId answerId, const std::set<Serialize::ParticipantId> &targets = {}) const;
            void writeRequest(OffsetPtr offset, void *data, Serialize::ParticipantId requester = 0, Serialize::MessageId requesterTransactionId = 0, Serialize::GenericMessageReceiver receiver = {}) const;
        
            Serialize::WriteMessage getSlaveRequestMessageTarget() const;


            
            template <typename Ty, typename... Args>
            void writeAction(Ty *field, Serialize::ParticipantId answerTarget, Serialize::MessageId answerId, Args &&...args) const
            {
                size_t componentIndex = std::remove_pointer_t<decltype(field->parent())>::component_index();
                OffsetPtr offset { this, field };
                typename Ty::action_payload data { std::forward<Args>(args)... };
                writeAction(offset, componentIndex, &data, answerTarget, answerId, {});
            }

            template <typename Ty, typename... Args>
            void writeRequest(Ty *field, Serialize::ParticipantId requester, Serialize::MessageId requesterTransactionId, Args &&...args) const
            {
                OffsetPtr offset { this, field };
                typename Ty::request_payload data { std::forward<Args>(args)... };
                writeRequest(offset, &data, requester, requesterTransactionId);
            }

            template <typename Ty, typename... Args>
            void writeRequest(Ty *field, Serialize::GenericMessageReceiver receiver, Args &&...args) const
            {
                OffsetPtr offset { this, field };
                typename Ty::request_payload data { std::forward<Args>(args)... };
                writeRequest(offset, &data, 0, 0, std::move(receiver));
            }

            template <typename T>
            friend struct Serialize::Syncable;

        };
    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::EntityComponentBase)