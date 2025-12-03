#include "../../scenelib.h"

#include "entityptr.h"

#include "Meta/serialize/streams/streamresult.h"

#include "Meta/serialize/operations.h"

#include "entityhandle.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        Serialize::StreamResult tag_invoke(Serialize::apply_map_t, EntityPtr &ptr, Serialize::CallerHierarchyFormattedSerializeStream in, bool success)
        {
            Scene::Entity::EntityHandle *h = std::exchange(reinterpret_cast<Scene::Entity::EntityHandle *&>(ptr), nullptr);
            STREAM_PROPAGATE_ERROR(Serialize::apply_map(h, in, success));
            if (h) {
                ptr = h->ptr();
            }
            return {};
        }

    }
}
namespace Serialize {

    StreamResult Operations<Scene::Entity::EntityPtr>::read(CallerHierarchyFormattedSerializeStream in, Scene::Entity::EntityPtr &e, const char *name)
    {
        e = {};
        return Serialize::read(in, reinterpret_cast<Scene::Entity::EntityHandle *&>(e), name);
    }

    void Operations<Scene::Entity::EntityPtr>::write(CallerHierarchyFormattedSerializeStream out, const Scene::Entity::EntityPtr &e, const char *name)
    {
        if (!e.access([&](Scene::Entity::Entity &e) {
                Serialize::write(out, &e.handle(), name);
            })) {
            Serialize::write(out, static_cast<Scene::Entity::EntityHandle *>(nullptr), name);
        }
    }

    StreamResult Operations<Scene::Entity::EntityPtr>::visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
    }

}
}
