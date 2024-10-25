#pragma once

#include "Meta/keyvalue/objectptr.h"

#include "Meta/serialize/container/serializablecontainer.h"

#include "Meta/serialize/hierarchy/syncableunit.h"

#include "Generic/container/mutable_set.h"

#include "Modules/uniquecomponent/component_index.h"

#include "entitycomponentptr.h"

#include "Generic/customfunctors.h"

#include "Generic/execution/lifetime.h"

#include "Madgine/debug/debuggablesender.h"

#include "Interfaces/log/logsenders.h"

#include "Madgine/behavior.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Entity : Serialize::SyncableUnit<Entity> {
            SERIALIZABLEUNIT(Entity)

            //Entity(const Entity &, bool local);
            //            Entity(Entity &&, bool local);
            Entity(Entity &&) = default;

            Entity(SceneContainer &container, const std::string &name);
            Entity(const Entity &) = delete;
            ~Entity();

            Entity &operator=(Entity &&other);

            void remove();

            const std::string &key() const;

            const std::string &name() const;

            template <typename T>
            EntityComponentPtr<T> addComponent(const ObjectPtr &table = {})
            {
                return EntityComponentPtr<T> { addComponent(UniqueComponent::component_index<T>(), table) };
            }

            template <typename T>
            void removeComponent()
            {
                removeComponent(component_index<T>());
            }

            template <typename T>
            EntityComponentPtr<T> getComponent()
            {
                return static_cast<EntityComponentPtr<T>>(getComponent(UniqueComponent::component_index<T>()));
            }

            template <typename T>
            EntityComponentPtr<const T> getComponent() const
            {
                return static_cast<EntityComponentPtr<const T>>(getComponent(UniqueComponent::component_index<T>()));
            }

            EntityComponentPtr<EntityComponentBase> getComponent(uint32_t i);
            EntityComponentPtr<const EntityComponentBase> getComponent(uint32_t i) const;
            EntityComponentPtr<EntityComponentBase> getComponent(std::string_view name);
            EntityComponentPtr<const EntityComponentBase> getComponent(std::string_view name) const;

            struct MADGINE_SCENE_EXPORT Helper {
                Entity *mEntity;
                EntityComponentPtr<EntityComponentBase> operator()(const EntityComponentOwningHandle<EntityComponentBase> &p) const;
            };
            decltype(auto) components()
            {
                return mComponents | std::views::transform(Helper { this });
            }

            template <typename T>
            bool hasComponent()
            {
                return hasComponent(UniqueComponent::component_index<T>());
            }

            bool hasComponent(size_t i);
            bool hasComponent(std::string_view name);

            EntityComponentPtr<EntityComponentBase> addComponent(std::string_view name, const ObjectPtr &table = {});
            EntityComponentPtr<EntityComponentBase> addComponent(size_t i, const ObjectPtr &table = {});
            void removeComponent(std::string_view name);
            void removeComponent(size_t i);
            void clearComponents();
            void relocateComponent(EntityComponentHandle<EntityComponentBase> newIndex);

            struct EntityScope {

                template <typename Rec>
                struct receiver : Execution::algorithm_receiver<Rec> {

                    template <typename O>
                    friend BehaviorError tag_invoke(get_binding_d_t, receiver &rec, std::string_view name, O &out)
                    {
                        if (name == "Entity") {
                            out = rec.mEntity;
                            return {};
                        } else if (name == "Scene"){
                            out = &rec.mEntity->sceneMgr();
                            return {};
                        } else {
                            return get_binding_d(rec.mRec, name, out);
                        }
                    }

                    Entity *mEntity;
                };

                template <typename Inner>
                struct sender : Execution::algorithm_sender<Inner> {
                    template <typename Rec>
                    friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
                    {
                        return Execution::algorithm_state<Inner, receiver<Rec>> { std::forward<Inner>(sender.mSender), std::forward<Rec>(rec), sender.mEntity };
                    }

                    Entity *mEntity;
                };

                template <typename Inner>
                friend auto operator|(Inner &&inner, EntityScope &&scope)
                {
                    return sender<Inner> { { {}, std::forward<Inner>(inner) }, scope.mEntity };
                }

                Entity *mEntity;
            };

            template <typename Sender>
            void addBehavior(Sender &&sender)
            {
                Debug::ContextInfo *context = &Debug::Debugger::getSingleton().createContext();
                mLifetime.attach(std::forward<Sender>(sender) | EntityScope { this } | Execution::with_debug_location<Execution::SenderLocation>() | Execution::with_sub_debug_location(context) | Log::log_error());
                mBehaviorContexts.emplace_back(context);
            }

            const std::vector<Debug::ContextInfo *> &behaviorContexts();

            void handleEntityEvent(const typename std::set<EntityComponentOwningHandle<EntityComponentBase>>::iterator &it, int op);

            SceneManager &sceneMgr() const;

            SceneContainer &container();
            const SceneContainer &container() const;

            friend struct SyncableEntityComponentBase;
            friend struct Scene::SceneContainer;

        public:
            std::string mName;

        private:
            Serialize::StreamResult readComponent(Serialize::FormattedSerializeStream &in, EntityComponentOwningHandle<EntityComponentBase> &handle);
            const char *writeComponent(Serialize::FormattedSerializeStream &out, const EntityComponentOwningHandle<EntityComponentBase> &comp) const;

            SERIALIZABLE_CONTAINER(mComponents, mutable_set<EntityComponentOwningHandle<EntityComponentBase>, std::less<>>, ParentFunctor<&Entity::handleEntityEvent>);

            SceneContainer &mContainer;

            Execution::Lifetime mLifetime;

            std::vector<Debug::ContextInfo *> mBehaviorContexts;
        };

    }
}
}
