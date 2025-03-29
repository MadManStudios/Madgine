#pragma once

#include "scenecomponentcollector.h"

#include "Madgine/app/globalapibase.h"
#include "Madgine/app/globalapicollector.h"
#include "Meta/serialize/container/noparent.h"

#include "Modules/threading/datamutex.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "entity/entitycomponentcollector.h"

#include "Generic/intervalclock.h"

#include "Modules/threading/customclock.h"

#include "scenecontainer.h"

#include "Madgine/debug/debuggablelifetime.h"

namespace Engine {
namespace Scene {
    struct MADGINE_SCENE_EXPORT SceneManager : Serialize::TopLevelUnit<SceneManager>,
                                               App::GlobalAPI<SceneManager> {

        using Self = SceneManager;

        SceneManager(App::Application &app);
        SceneManager(const SceneManager &) = delete;
        ~SceneManager();

        virtual std::string_view key() const override;

        void updateFrame(Closure<ByteBufferImpl<Matrix4[]>(Entity::SkeletonPtr)> callback);

        void clear();

        void pause();
        bool unpause();
        bool isPaused() const;
        const Threading::CustomClock &clock() const;

        IntervalClock<Threading::CustomTimepoint> &simulationClock();
        IntervalClock<Threading::CustomTimepoint> &animationClock();

        SceneContainer &container(std::string_view name);

        template <typename T>
        T &getComponent()
        {
            return static_cast<T &>(getComponent(UniqueComponent::component_index<T>()));
        }
        SceneComponentBase &getComponent(size_t i);
        size_t getComponentCount();
                
        void startLifetime() override;
        bool endLifetime();

        Debug::DebuggableLifetime<get_binding_d> &lifetime(); 

        template <typename Sender>
        void addBehavior(Sender &&sender)
        {
            mLifetime.attach(std::forward<Sender>(sender) | with_constant_binding<"Scene">(this) | Log::log_result());
        }

        void addAnimation(Entity::AnimationState *animation);
        bool stopAnimation(Entity::AnimationState *animation);

        Threading::DataMutex &mutex();

        template <typename T>
        Entity::EntityComponentList<T> &entityComponentList()
        {
            return static_cast<Entity::EntityComponentList<T> &>(*mEntityComponentLists.at(UniqueComponent::component_index<T>()));
        }

        Entity::EntityComponentListBase &entityComponentList(size_t index)
        {
            return *mEntityComponentLists.at(index);
        }

        const Entity::EntityComponentListBase &entityComponentList(size_t index) const
        {
            return *mEntityComponentLists.at(index);
        }

    protected:
        virtual Threading::Task<bool> init() final;
        virtual Threading::Task<void> finalize() final;

    private:
        struct Clock : Threading::CustomClock {
            virtual std::chrono::steady_clock::time_point get(std::chrono::steady_clock::time_point timepoint) const override;
            virtual std::chrono::steady_clock::time_point revert(std::chrono::steady_clock::time_point timepoint) const override;

            std::chrono::steady_clock::duration mPauseAcc = std::chrono::steady_clock::duration::zero();
            std::chrono::steady_clock::time_point mPauseStart;
            std::atomic<size_t> mPauseStack = 0;
        } mClock;

        friend struct SceneContainer;

        Threading::DataMutex mMutex;
        DEBUGGABLE_LIFETIME(mLifetime, get_binding_d);

        IntervalClock<Threading::CustomTimepoint> mSimulationClock;
        IntervalClock<Threading::CustomTimepoint> mAnimationClock;
        IntervalClock<std::chrono::steady_clock::time_point> mFrameClock;

        std::mutex mAnimationMutex;
        std::vector<Entity::AnimationState *> mAnimationStates;

        Entity::EntityComponentListContainer<std::vector<Placeholder<0>>> mEntityComponentLists;

    public:
        MEMBER_OFFSET_CONTAINER(mSceneComponents, , SceneComponentContainer<Serialize::SerializableContainer<std::set<Placeholder<0>, KeyCompare<Placeholder<0>>>, NoOpFunctor>>);

        struct ContainerData {
            ContainerData(SceneManager &manager);
            ContainerData(ContainerData &&) = delete;

            Serialize::NoParent<SceneContainer> mContainer;
        };
        std::map<std::string, ContainerData> mContainers;
    };

}
}

REGISTER_TYPE(Engine::Scene::SceneManager)
REGISTER_TYPE(Engine::Serialize::NoParent<Engine::Scene::SceneManager>)