#pragma once

#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "Generic/container/freelistcontainer.h"
#include "entity/entity.h"

#include "Meta/serialize/container/syncablecontainer.h"

#include "Generic/execution/signalfunctor.h"

#include "Generic/projections.h"

namespace Engine {
namespace Scene {

    struct MADGINE_SCENE_EXPORT SceneContainer : Serialize::TopLevelUnit<SceneContainer> {
        SERIALIZABLEUNIT(SceneContainer)

        using EntityContainer = std::list<Entity::Entity>;

        SceneContainer(SceneManager &sceneMgr);

        Entity::EntityPtr createEntity(const std::string &name = "",
            const std::function<void(Entity::Entity &)> &init = {});
        void createEntityAsyncImpl(Serialize::GenericMessageReceiver receiver, const std::string &name = "",
            std::function<void(Entity::Entity &)> init = {});
        ASYNC_STUB(createEntityAsync, createEntityAsyncImpl, Serialize::make_message_sender<Entity::EntityPtr>);
                                
        void startLifetime();
        void endLifetime();

        Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

        Entity::EntityPtr findEntity(const std::string &name);
        void remove(Entity::Entity *e);
        void remove(Entity::EntityPtr e);
        void clear();

        Execution::SignalStub<const EntityContainer::iterator &, int> &entitiesSignal();

        static std::string generateUniqueName();

        auto entities() {
            return mEntities | std::views::transform(&Entity::Entity::pointer);
        }

        Threading::DataMutex &mutex();

        SceneManager &sceneMgr() const;        

        friend struct SceneManager;

    private:
        Entity::EntityPtr spawnEntity(const std::string &name, const std::function<void(Entity::Entity &)> &init);
        
        friend struct Entity::Entity;

        DEBUGGABLE_LIFETIME(mLifetime, Behavior::get_named_d);

        SceneManager &mManager;

        std::string mName;

    private:
        Serialize::StreamResult readEntity(Serialize::CallerHierarchyFormattedSerializeStream in, OutRef<SceneContainer> &mgr, std::string &name);
        std::tuple<SceneContainer &, std::string> createEntityData(const std::string &name);
        const char *writeEntity(Serialize::CallerHierarchyFormattedSerializeStream out, const Entity::Entity &entity) const;

        SYNCABLE_CONTAINER(mEntities, EntityContainer, Execution::SignalFunctor<const EntityContainer::iterator &, int>);

    };

}
}