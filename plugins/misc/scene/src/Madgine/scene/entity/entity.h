#pragma once

#include "Generic/container/mutable_set.h"
#include "Generic/customfunctors.h"

#include "Interfaces/log/logsenders.h"

#include "Meta/serialize/container/serializablecontainer.h"
#include "Meta/serialize/hierarchy/syncableunit.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Madgine/behavior/behaviorlist.h"
#include "Madgine/debug/debuggablelifetime.h"

#include "entitycomponenthandle.h"
#include "entityptr.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Entity : Serialize::SerializableUnitBase {
            SERIALIZABLEUNIT(Entity)

            // Entity(const Entity &, bool local);
            //             Entity(Entity &&, bool local);
            Entity(Entity &&) = delete;

            Entity(EntityHandle &handle, SceneContainer &container, const std::string &name);
            Entity(const Entity &) = delete;
            ~Entity();

            void startLifetime();
            void endLifetime();

            EntityPtr pointer();
            EntityHandle &handle();

            Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

            const std::string &key() const;

            const std::string &name() const;

            template <typename T>
            T *addComponent()
            {
                return static_cast<T *>(addComponent(UniqueComponent::component_index<T>()));
            }

            template <typename T>
            void removeComponent()
            {
                removeComponent(component_index<T>());
            }

            template <typename T>
            T *getComponent()
            {
                return static_cast<T *>(getComponent(UniqueComponent::component_index<T>()));
            }

            template <typename T>
            const T *getComponent() const
            {
                return static_cast<const T *>(getComponent(UniqueComponent::component_index<T>()));
            }

            EntityComponentBase *getComponent(uint32_t i);
            const EntityComponentBase *getComponent(uint32_t i) const;
            EntityComponentBase *getComponent(std::string_view name);
            const EntityComponentBase *getComponent(std::string_view name) const;

            const mutable_set<EntityComponentHandle, std::less<>> &components()
            {
                return mComponents;
            }

            template <typename T>
            bool hasComponent()
            {
                return hasComponent(UniqueComponent::component_index<T>());
            }

            bool hasComponent(size_t i);
            bool hasComponent(std::string_view name);

            EntityComponentBase *addComponent(std::string_view name);
            EntityComponentBase *addComponent(size_t i);
            void removeComponent(std::string_view name);
            void removeComponent(size_t i);
            void clearComponents();

            template <typename Sender>
            void addBehavior(Sender &&sender)
            {
                mLifetime.attach(std::forward<Sender>(sender) | Log::log_result());
            }

            auto lifetimeSender()
            {
                return mLifetime.bound(mSelf, *this, Functor<&Entity::dtor> {}) | Behavior::with_named<"Entity">(mSelf);
            }

            void handleEntityEvent(const typename mutable_set<EntityComponentHandle, std::less<>>::iterator &it, int op);

            SceneManager &sceneMgr() const;

            SceneContainer &container();
            const SceneContainer &container() const;

            Behavior::BehaviorList &behaviors();

            friend struct Scene::SceneContainer;

            void onActivate(Serialize::CallbackTiming timing, bool active, bool existenceChanged);

        protected:
            Debug::DebuggableLifetimeBase &lifetimeBase();

            static void dtor(Entity &e);

        public:
            std::string mName;

        private:
            Serialize::StreamResult readComponent(Serialize::CallerHierarchyFormattedSerializeStream in, uint32_t &type, OutRef<EntityComponentBase> &ptr);
            const char *writeComponent(Serialize::CallerHierarchyFormattedSerializeStream out, const EntityComponentHandle &p) const;

            SERIALIZABLE_CONTAINER(mComponents, mutable_set<EntityComponentHandle, std::less<>>, ParentFunctor<&Entity::handleEntityEvent>);

            EntityHandle &mHandle;

            SceneContainer &mContainer;

            DEBUGGABLE_LIFETIME(mLifetime, Behavior::get_named_d);

            EntityPtr mSelf;

            Behavior::BehaviorList mBehaviors;
        };

    }
}
}
