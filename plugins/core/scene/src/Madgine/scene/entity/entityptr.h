#pragma once

#include "Generic/bits/field.h"
#include "Generic/bits/ptr.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT EntityPtr {

            EntityPtr() = default;
            ~EntityPtr();

            EntityPtr(const EntityPtr &other);
            EntityPtr(Entity *entity);

            EntityPtr &operator=(const EntityPtr &other);
            EntityPtr &operator=(EntityPtr &&other);

            Entity *operator->() const;

            void update() const;
            void reset();
            //operator bool() const;

            operator Entity *() const;
            Entity *get() const;

            bool operator==(const EntityPtr &other) const;
            bool operator==(Entity *other) const;
            std::strong_ordering operator<=>(const EntityPtr &other) const;

            bool isDead() const;

        private:
            struct ControlBlockDummy;
            ControlBlockDummy getBlock() const;

            friend struct Serialize::Operations<EntityPtr>;

            META_EXPORT friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, EntityPtr &unit, Serialize::FormattedSerializeStream &in, bool success, const CallerHierarchyBasePtr &hierarchy);


        private:
            union {
                BitPtr<Entity, 2> mEntity = nullptr;
                mutable BitField<1, 1> mHoldsRef; // Make sure not to conflict with Serialize::UnitIdTag::SYNCABLE
            };
        };

    }
}
namespace Serialize {

    template <>
    struct MADGINE_SCENE_EXPORT Operations<Scene::Entity::EntityPtr> {
        static StreamResult read(FormattedSerializeStream &in, Scene::Entity::EntityPtr &ptr, const char *name = nullptr);
        static void write(Serialize::FormattedSerializeStream &out, const Scene::Entity::EntityPtr &ptr, const char *name = nullptr);
    };

}
}