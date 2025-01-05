#include "../scenelib.h"

#include "scenemanager.h"

#include "Meta/serialize/hierarchy/serializableids.h"

#include "scenecomponentbase.h"

#include "Madgine/app/application.h"

#include "Modules/debug/profiler/profile.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "entity/entity.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "entity/entitycomponentlistbase.h"

#include "Meta/serialize/configs/controlled.h"

#include "Meta/serialize/configs/guard.h"

#include "Modules/threading/awaitables/awaitablesender.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/promise.h"

#include "behavior/animation.h"
#include "entity/components/skeleton.h"

UNIQUECOMPONENT(Engine::Scene::SceneManager);

METATABLE_BEGIN(Engine::Scene::SceneManager)
//TODO
//SYNCABLEUNIT_MEMBERS()
MEMBER(mSceneComponents)
//MEMBER(mContainers)
METATABLE_END(Engine::Scene::SceneManager)

/* static Engine::Threading::DataMutex::Lock static_lock(Engine::Scene::SceneManager *mgr)
{
    return mgr->mutex().lock(Engine::AccessMode::WRITE);
}

SERIALIZETABLE_BEGIN(Engine::Scene::SceneManager,
    Engine::Serialize::CallableGuard<&static_lock>)
FIELD(mSceneComponents, Serialize::ControlledConfig<KeyCompare<std::unique_ptr<Engine::Scene::SceneComponentBase>>>)
SERIALIZETABLE_END(Engine::Scene::SceneManager)*/

namespace Engine {
namespace Scene {

    SceneManager::ContainerData::ContainerData(SceneManager &manager)
        : mContainer(manager)
    {
    }

    SceneManager::SceneManager(App::Application &app)
        : VirtualScope(app)
        , mSceneComponents(*this)
        , mMutex("SceneData")
        , mFrameClock(std::chrono::steady_clock::now())
        , mAnimationClock(mClock.now())
        , mSimulationClock(mClock.now())
    {
    }

    SceneManager::~SceneManager()
    {
    }

    std::string_view SceneManager::key() const
    {
        return "SceneManager";
    }

    Threading::Task<bool> SceneManager::init()
    {
        for (const std::unique_ptr<SceneComponentBase> &component : mSceneComponents) {
            if (!co_await component->callInit())
                co_return false;
        }

        taskQueue()->queue([this]() -> Threading::Task<void> {
            while (taskQueue()->running()) {
                mSimulationClock.tick(mClock.now());
                co_await std::chrono::microseconds { 1000000 / 60 };
            }
        });

        startLifetime();

        co_return true;
    }

    Threading::Task<void> SceneManager::finalize()
    {
        //assert(mEntities.empty());
        //assert(mLocalEntities.empty());

        for (const std::unique_ptr<SceneComponentBase> &component : mSceneComponents) {
            co_await component->callFinalize();
        }
    }

    SceneComponentBase &SceneManager::getComponent(size_t i)
    {
        return mSceneComponents.get(i);
    }

    size_t SceneManager::getComponentCount()
    {
        return mSceneComponents.size();
    }

    Threading::DataMutex &SceneManager::mutex()
    {
        return mMutex;
    }

    void SceneManager::updateFrame(Closure<ByteBufferImpl<Matrix4[]>(Entity::SkeletonPtr)> callback)
    {
        std::chrono::microseconds frameTimeSinceLastFrame = mFrameClock.tick(std::chrono::steady_clock::now());
        std::chrono::microseconds sceneTimeSinceLastFrame = mAnimationClock.tick(mClock.now());

        for (const std::unique_ptr<SceneComponentBase> &comp : mSceneComponents) {
            comp->updateFrame(frameTimeSinceLastFrame, sceneTimeSinceLastFrame);
        }

        std::vector<Entity::AnimationState *> doneAnimations;

        {
            std::unique_lock lock { mAnimationMutex };
            for (auto it = mAnimationStates.begin(); it != mAnimationStates.end();) {

                Entity::AnimationState *animation = *it;

                Entity::SkeletonPtr skeleton = animation->entity()->getComponent<Scene::Entity::Skeleton>();

                const Render::SkeletonDescriptor *data = skeleton->data();
                if (data) {
                    size_t boneCount = data->mBones.size();
                    auto buffer = callback(skeleton);

                    if (animation->updateRender(frameTimeSinceLastFrame, sceneTimeSinceLastFrame, buffer.mData)) {
                        doneAnimations.push_back(animation);
                        it = mAnimationStates.erase(it);
                    } else {
                        ++it;
                    }

                    /* if (Scene::Entity::Transform *transform = animation->entity()->getComponent<Scene::Entity::Transform>()) {
                    for (size_t i = 0; i < boneCount; ++i) {
                        Vector4 v1 = buffer[i].Transpose() * data->mBones[i].mOffsetMatrix.Inverse() * Vector4 { 0.0f, 0.0f, 0.0f, 1.0f };
                        Vector4 v2 = v1 + Vector4 { 0.0f, 1.0f, 0.0f, 0.0f };
                        if (data->mBones[i].mFirstChild) {
                            size_t j = data->mBones[i].mFirstChild;
                            v2 = buffer[j].Transpose() * data->mBones[j].mOffsetMatrix.Inverse() * Vector4 { 0.0f, 0.0f, 0.0f, 1.0f };
                        }

                        Im3D::Arrow3D(IM3D_LINES, 0.2f, v1.xyz(), v2.xyz(), { .mTransform = transform->matrix() });
                    }
                }*/
                } else {
                    ++it;
                }
            }
        }

        for (Entity::AnimationState* state : doneAnimations) {
            state->finish();
        }

    }

    void SceneManager::clear()
    {
        endLifetime();
        startLifetime();
    }

    void SceneManager::pause()
    {
        if (mClock.mPauseStack++ == 0) {
            mClock.mPauseStart = std::chrono::steady_clock::now();
        }
    }

    bool SceneManager::unpause()
    {
        if (--mClock.mPauseStack == 0) {
            mClock.mPauseAcc += std::chrono::steady_clock::now() - mClock.mPauseStart;
            return true;
        }
        return false;
    }

    bool SceneManager::isPaused() const
    {
        return mClock.mPauseStack > 0;
    }

    const Threading::CustomClock &SceneManager::clock() const
    {
        return mClock;
    }

    IntervalClock<Threading::CustomTimepoint> &SceneManager::simulationClock()
    {
        return mSimulationClock;
    }

    IntervalClock<Threading::CustomTimepoint> &SceneManager::animationClock()
    {
        return mAnimationClock;
    }

    SceneContainer &SceneManager::container(std::string_view name)
    {
        return mContainers.try_emplace(std::string { name }, *this).first->second.mContainer;
    }

    std::chrono::steady_clock::time_point SceneManager::Clock::get(std::chrono::steady_clock::time_point timepoint) const
    {
        return (mPauseStack > 0 ? mPauseStart : timepoint) - mPauseAcc;
    }

    std::chrono::steady_clock::time_point SceneManager::Clock::revert(std::chrono::steady_clock::time_point timepoint) const
    {
        return timepoint + mPauseAcc + (mPauseStack > 0 ? std::chrono::steady_clock::now() - mPauseStart : 0s);
    }

    void SceneManager::addAnimation(Entity::AnimationState *animation)
    {
        std::unique_lock lock { mAnimationMutex };
        mAnimationStates.push_back(animation);
    }

    bool SceneManager::stopAnimation(Entity::AnimationState *animation)
    {
        std::unique_lock lock { mAnimationMutex };
        return std::erase(mAnimationStates, animation) == 1;
    }

    void SceneManager::startLifetime()
    {
        mApp.lifetime().attach(mLifetime | with_constant_binding<"Scene">(this));
        for (ContainerData& container : kvValues(mContainers)) {
            container.mContainer.startLifetime();
        }
    }

    void SceneManager::endLifetime()
    {
        mLifetime.end();
    }

}
}
