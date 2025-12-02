#pragma once

#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "Generic/container/freelistcontainer.h"
#include "entity.h"

#include "Meta/serialize/container/syncablecontainer.h"

#include "Generic/execution/signalfunctor.h"

#include "Generic/projections.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityHandle : Serialize::SyncableUnit<EntityHandle> {

            friend struct SyncableEntityComponentBase;

            EntityHandle(SceneContainer &container, const std::string &name, std::function<void(Entity &)> init = {});

            const EntityPtr &ptr() const;

        private:
            EntityPtr mPtr;
        };

    }
}

namespace Serialize {

    template <typename... Configs>
    struct Operations<Scene::Entity::EntityHandle, Configs...> {

        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Scene::Entity::EntityHandle &handle, const char *name)
        {
            StreamResult result;
            Execution::access_binding(handle.ptr(), [&](Scene::Entity::Entity &entity) {
                result = SerializableDataPtr { &entity }.readState(in, name, true);
            });
            return result;
        }

        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Scene::Entity::EntityHandle &handle, const char *name)
        {
            bool success = Execution::access_binding(handle.ptr(), [&](Scene::Entity::Entity &entity) {
                SerializableDataConstPtr { &entity }.writeState(out, name, true);
            });
            assert(success);
        }

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor)
        {
            return Serialize::visitStream<Scene::Entity::Entity>(in, name, visitor);
        }
    };

}

}