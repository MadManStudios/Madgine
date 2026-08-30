#pragma once

#include "Generic/containers/freelistcontainer.h"
#include "Generic/execution/signalfunctor.h"
#include "Generic/projections.h"

#include "Meta/serialize/container/syncablecontainer.h"
#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "entity/entity.h"
#include "entity/entitydescriptor.h"
#include "entity/entityhandle.h"

namespace Engine {
namespace Scene {

    struct MADGINE_SCENE_EXPORT SceneContainer : Serialize::TopLevelUnit<SceneContainer> {
        SERIALIZABLEUNIT(SceneContainer)

        using EntityContainer = std::list<Entity::EntityHandle>;

        SceneContainer(SceneManager &sceneMgr);

        
        Execution::Future<Serialize::MessageResult, Entity::EntityPtr> createEntity(const std::string &name = "", Entity::EntityDescriptor init = {}, Entity::EntityPtr parent = {}, Closure<void(Entity::EntityPtr)> cb = {}, Closure<void(Serialize::MessageResult)> onError = {});

        void startLifetime();
        void endLifetime();

        void copy(const SceneContainer &other);

        Debug::DebuggableLifetime<Reflect::get_reflect_contextual> &lifetime();

        Entity::EntityPtr findEntity(const std::string &name);
        void remove(Entity::EntityPtr e);
        void clear();

        Execution::SignalStub<void, const EntityContainer::iterator &, int> &entitiesSignal();

        static std::string generateUniqueName();

        std::string_view name() const;

        auto entities()
        {
            return mEntities | std::views::transform(&Entity::EntityHandle::ptr);
        }

        Threading::DataMutex &mutex();

        SceneManager &sceneMgr() const;

        friend struct SceneManager;

    private:
        friend struct Entity::Entity;

        DEBUGGABLE_LIFETIME(mLifetime, Reflect::get_reflect_contextual);

        SceneManager &mManager;

        std::string mName;

    private:
        Serialize::StreamResult readEntity(Serialize::FormattedSerializeStream &in, OutRef<SceneContainer> &mgr, std::string &name);
        std::tuple<SceneContainer &, std::string, Entity::EntityDescriptor, Entity::EntityPtr> createEntityData(const std::string &name, Entity::EntityDescriptor init, Entity::EntityPtr parent);
        const char *writeEntity(Serialize::FormattedSerializeStream &out, const Entity::EntityHandle &handle) const;

        SYNCABLE_CONTAINER(mEntities, EntityContainer, Execution::SignalFunctor<void, const EntityContainer::iterator &, int>);
    };

}
}