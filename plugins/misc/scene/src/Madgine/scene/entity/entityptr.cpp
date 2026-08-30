#include "../../scenelib.h"

#include "entityptr.h"

#include "Meta/serialize/operations.h"
#include "Meta/serialize/streams/streamresult.h"

#include "entityhandle.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        std::strong_ordering EntityPtr::operator<=>(const EntityPtr &other) const
        {
            return ptr() <=> other.ptr();
        }

        bool EntityPtr::operator==(const EntityPtr &other) const
        {
            return ptr() == other.ptr();
        }

        EntityPtr::operator bool() const
        {
            return static_cast<bool>(ptr());
        }

        void EntityPtr::fromEntity(Entity &e)
        {
            Base::operator=(static_cast<Base &&>(e.pointer()));
        }

        Serialize::StreamResult tag_invoke(Serialize::apply_map_t, EntityPtr &ptr, Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context)
        {
            Scene::Entity::EntityHandle *h = std::exchange(reinterpret_cast<Scene::Entity::EntityHandle *&>(ptr), nullptr);
            STREAM_PROPAGATE_ERROR(Serialize::apply_map(h, in, success, context));
            if (h) {
                ptr = h->ptr();
            }
            return {};
        }

    }
}
namespace Serialize {

    StreamResult Operations<Scene::Entity::EntityPtr>::read(FormattedSerializeStream &in, Scene::Entity::EntityPtr &e, const char *name, ContextPtr context)
    {
        e = {};
        return Serialize::read(in, reinterpret_cast<Scene::Entity::EntityHandle *&>(e), name, context);
    }

    void Operations<Scene::Entity::EntityPtr>::write(FormattedSerializeStream &out, const Scene::Entity::EntityPtr &e, const char *name, ContextPtr context)
    {
        if (!Execution::access_binding(e, [&](Scene::Entity::Entity &e) {
                Serialize::write(out, &e.handle(), name);
            })) {
            Serialize::write(out, static_cast<Scene::Entity::EntityHandle *>(nullptr), name, context);
        }
    }

    StreamResult Operations<Scene::Entity::EntityPtr>::visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
    {
        throw 0;
    }

}
}
