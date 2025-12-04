#pragma once

#include "Generic/container/freelistcontainer.h"
#include "Generic/execution/signalfunctor.h"
#include "Generic/projections.h"

#include "Meta/serialize/container/syncablecontainer.h"
#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "entity.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityHandle : Serialize::SyncableUnit<EntityHandle> {

            friend struct SyncableEntityComponentBase;

            EntityHandle(SceneContainer &container, const std::string &name, std::function<void(Entity &)> init = {});

            const EntityPtr &ptr() const;

        private:
            template <typename T, typename... Configs>
            friend struct Serialize::Operations;

            EntityPtr mPtr;
        };

    }
}

namespace Serialize {

    template <typename... Configs>
    struct Operations<Scene::Entity::EntityHandle, Configs...> {

        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Scene::Entity::EntityHandle &handle, const char *name)
        {
            STREAM_PROPAGATE_ERROR(handle.readId(in, name));
            StreamResult result;
            handle.ptr().access([&](Scene::Entity::Entity &entity) {
                result = SerializableDataPtr { &entity }.readState(in, name, true);
            });
            return result;
        }

        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Scene::Entity::EntityHandle &handle, const char *name)
        {
            handle.writeId(out, name);
            bool success = handle.ptr().access([&](Scene::Entity::Entity &entity) {
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