#pragma once

#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "Generic/container/refcountedcontainer.h"
#include "entity/entity.h"

#include "Meta/serialize/container/syncablecontainer.h"

#include "entity/entityptr.h"

#include "Generic/execution/signalfunctor.h"

#include "Generic/projections.h"

namespace Engine {
namespace Scene {

    struct MADGINE_SCENE_EXPORT SceneContainer : Serialize::TopLevelUnit<SceneContainer> {
        SERIALIZABLEUNIT(SceneContainer)

        using EntityContainer = RefcountedContainer<std::deque<Entity::Entity>>;
        using ControlBlock = typename EntityContainer::ControlBlock;

        SceneContainer(SceneManager &sceneMgr);

        Entity::EntityPtr createEntity(const std::string &name = "",
            const std::function<void(Entity::Entity &)> &init = {});
        void createEntityAsyncImpl(Serialize::GenericMessageReceiver receiver, const std::string &name = "",
            std::function<void(Entity::Entity &)> init = {});
        ASYNC_STUB(createEntityAsync, createEntityAsyncImpl, Serialize::make_message_sender<Entity::EntityPtr>);

        Entity::EntityPtr findEntity(const std::string &name);
        void remove(Entity::Entity *e);
        void clear();

        Execution::SignalStub<const EntityContainer::iterator &, int> &entitiesSignal();

        static std::string generateUniqueName();

        auto entities() {
            return mEntities | std::views::transform(projectionAddressOf);
        }

        Threading::DataMutex &mutex();

        SceneManager &sceneMgr() const;        

        friend struct SceneManager;

    private:
        friend struct Entity::Entity;

        Execution::Lifetime mLifetime;

        SceneManager &mManager;

        std::string mName;

    private:
        Serialize::StreamResult readEntity(Serialize::FormattedSerializeStream &in, OutRef<SceneContainer> &mgr, std::string &name);
        std::tuple<SceneContainer &, std::string> createEntityData(const std::string &name);
        const char *writeEntity(Serialize::FormattedSerializeStream &out, const Entity::Entity &entity) const;

        SYNCABLE_CONTAINER(mEntities, EntityContainer, Execution::SignalFunctor<const EntityContainer::iterator &, int>);

        struct EntityHelper {
            Entity::EntityPtr operator()(Entity::Entity &ref) const
            {
                return { &ref };
            }
        };
    };

}
}