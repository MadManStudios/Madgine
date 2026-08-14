#pragma once

#include "Generic/containers/freelistcontainer.h"
#include "Generic/execution/signalfunctor.h"
#include "Generic/projections.h"

#include "Meta/serialize/container/syncablecontainer.h"
#include "Meta/serialize/hierarchy/toplevelunit.h"

#include "entity.h"
#include "entitydescriptor.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityHandle : Serialize::SyncableUnit<EntityHandle> {

            friend struct SyncableEntityComponentBase;

            EntityHandle(SceneContainer &container, const std::string &name, const EntityDescriptor &init = {});

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
        template <typename Context>
        static StreamResult read(Serialize::FormattedSerializeStream &in, Scene::Entity::EntityHandle &handle, const char *name, Context &&context)
        {
            STREAM_PROPAGATE_ERROR(handle.readId(in, name));
            StreamResult result;
            Execution::access_binding(handle.ptr(), [&](Scene::Entity::Entity &entity) {
                result = SerializableDataPtr { &entity }.readState(in, name, true, context);
            });
            return result;
        }

        template <typename Context>
        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityHandle &handle, const char *name, Context &&context)
        {
            handle.writeId(out, name);
            [[maybe_unused]] bool success = Execution::access_binding(handle.ptr(), [&](Scene::Entity::Entity &entity) {
                SerializableDataConstPtr { &entity }.writeState(out, name, true, context);
            });
            assert(success);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor)
        {
            return Serialize::visitStream<Scene::Entity::Entity>(in, name, visitor);
        }
    };

}

}