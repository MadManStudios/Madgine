#pragma once

#include "Generic/execution/binding.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityPtr : Execution::BindingPtr<Entity &> {

            template <Execution::Binding<Entity&> Binding>
            EntityPtr& operator=(Binding&& binding) {
                Execution::BindingPtr<Entity &>::operator=(std::forward<Binding>(binding));
                return *this;
            }

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