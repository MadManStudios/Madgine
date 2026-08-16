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

            friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, EntityPtr &, Serialize::FormattedSerializeStream &, bool, Serialize::ContextPtr);

            template <typename... Configs, typename Context>
            friend void tag_invoke(Serialize::set_active_t<Configs...>, EntityPtr &, bool, bool, Context &&)
            {
            }

            template <Concepts::DecayedOneOf<EntityPtr> T, typename Callable, typename Context>
            friend Reflect::Result tag_invoke(Reflect::call_t<T> call, Callable &&callable, const Reflect::Value &arg, Context &&context)
            {
                if (Reflect::Value_isNull(arg)) {
                    return callable(EntityPtr {}, context);
                }
                return tag_invoke(Reflect::call_t<Reflect::ScopeBinding> {}, [&](const Reflect::ScopeBinding &binding, Context &context) { return callable(EntityPtr { binding.typed<Entity &>() }, context); }, arg, context);                
            }
        };

    }
}

namespace Serialize {
    template <>
    struct Operations<Scene::Entity::EntityPtr> {
        static StreamResult read(FormattedSerializeStream &in, Scene::Entity::EntityPtr &e, const char *name, ContextPtr context);
        static void write(FormattedSerializeStream &out, const Scene::Entity::EntityPtr &e, const char *name, ContextPtr context);
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth);
    };
}

}