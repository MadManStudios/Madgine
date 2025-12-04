#pragma once

#include "Generic/intervalclock.h"

#include "Meta/math/color3.h"
#include "Meta/math/vector3.h"
#include "Meta/serialize/container/noparent.h"

#include "Modules/threading/customclock.h"
#include "Modules/threading/datamutex.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/app/globalapibase.h"
#include "Madgine/app/globalapicollector.h"
#include "Madgine/debug/debuggablelifetime.h"

#include "entity/entitycomponentcollector.h"
#include "scenecomponentcollector.h"
#include "scenecontainer.h"

namespace Engine {
namespace Scene {
    struct MADGINE_SCENE_EXPORT SceneManager : Serialize::TopLevelUnit<SceneManager>,
                                               App::GlobalAPI<SceneManager> {

        using Self = SceneManager;

        SceneManager(App::Application &app);
        SceneManager(const SceneManager &) = delete;
        ~SceneManager();

        virtual std::string_view key() const override;

        void updateFrame(Closure<ByteBufferImpl<Matrix4[]>(Entity::Skeleton *)> callback);

        void clear();

        void pause();
        bool unpause();
        bool isPaused() const;
        const Threading::CustomClock &clock() const;

        IntervalClock<Threading::CustomTimepoint> &simulationClock();
        IntervalClock<Threading::CustomTimepoint> &animationClock();

        SceneContainer &container(std::string_view name);
        auto containers()
        {
            return mContainers | std::views::transform([](std::pair<const std::string, ContainerData> &p) -> SceneContainer & {
                return p.second.mContainer;
            });
        }

        template <typename T>
        T &getComponent()
        {
            return static_cast<T &>(getComponent(UniqueComponent::component_index<T>()));
        }
        SceneComponentBase &getComponent(size_t i);
        size_t getComponentCount();

        void startLifetime() override;
        bool endLifetime();

        Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

        template <typename Sender>
        void addBehavior(Sender &&sender)
        {
            mLifetime.attach(std::forward<Sender>(sender) | Log::log_result());
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
        DEBUGGABLE_LIFETIME(mLifetime, Behavior::get_named_d);

        IntervalClock<Threading::CustomTimepoint> mSimulationClock;
        IntervalClock<Threading::CustomTimepoint> mAnimationClock;
        IntervalClock<std::chrono::steady_clock::time_point> mFrameClock;

        std::mutex mAnimationMutex;
        std::vector<Entity::AnimationState *> mAnimationStates;

        UniqueComponent::Container<std::vector<std::unique_ptr<Entity::EntityComponentListBase>>, Entity::EntityComponentRegistry, Entity::EntityComponentListBase> mEntityComponentLists;

    public:
        MEMBER_OFFSET_CONTAINER(mSceneComponents, , SceneComponentContainer<Serialize::SerializableContainer<std::set<Placeholder<0>, KeyCompare<Placeholder<0>>>, NoOpFunctor>>);

        struct ContainerData {
            ContainerData(SceneManager &manager);
            ContainerData(ContainerData &&) = delete;

            Serialize::NoParent<SceneContainer> mContainer;
        };
        std::map<std::string, ContainerData> mContainers;

        Color3 mAmbientLightColor = { 1.0f, 1.0f, 1.0f };
        NormalizedVector3 mAmbientLightDirection = { -0.0f, -1.0f, 1.5f };
        bool mAmbientLightOrthographic = false;
    };

}
}