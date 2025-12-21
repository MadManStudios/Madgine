#pragma once

#include "Generic/execution/binding.h"

#include "Generic/execution/lifetime.h"

#include "Madgine/behavior/named.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT EntityPtr : Execution::Lifetime<Behavior::get_named_d>::BindingPoint<Execution::ConstantBinding<Entity&>> {

            using Base = Execution::Lifetime<Behavior::get_named_d>::BindingPoint<Execution::ConstantBinding<Entity&>>;

            EntityPtr() = default;
            EntityPtr(const EntityPtr &) = default;
            EntityPtr(EntityPtr &&) = default;

            using Base::Base;

            template <Execution::Binding<Entity &> Binding>
                requires(!std::same_as<std::remove_cvref_t<Binding>, EntityPtr>)
            EntityPtr(Binding &&binding)
            {
                Execution::access_binding(std::forward<Binding>(binding), [this](Entity &e) {
                    fromEntity(e);
                });
            }

            EntityPtr &operator=(const EntityPtr &other) = default;

            EntityPtr &operator=(EntityPtr &&other) = default;

            std::strong_ordering operator<=>(const EntityPtr &other) const;
            bool operator==(const EntityPtr &other) const;

            explicit operator bool() const;

            template <Execution::Binding<Entity &> Binding>
                requires(!std::same_as<std::remove_cvref_t<Binding>, EntityPtr>)
            EntityPtr &operator=(Binding &&binding)
            {
                if (!Execution::access_binding(std::forward<Binding>(binding), [this](Entity &e) {
                        fromEntity(e);
                    })) {
                    *this = {};
                }
                return *this;
            }

        private:
            void fromEntity(Entity &e);

            friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, EntityPtr &, Serialize::CallerHierarchyFormattedSerializeStream, bool);

            template <typename... Configs>
            friend void tag_invoke(Serialize::set_active_t<Configs...>, EntityPtr &, bool, bool, const CallerHierarchyBasePtr &)
            {
            }
        };

    }
}

namespace Serialize {
    template <>
    struct Operations<Scene::Entity::EntityPtr> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, Scene::Entity::EntityPtr &e, const char *name);
        static void write(CallerHierarchyFormattedSerializeStream out, const Scene::Entity::EntityPtr &e, const char *name);
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };
}

}