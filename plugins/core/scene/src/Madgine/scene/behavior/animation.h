#pragma once

#include "Madgine/animationloader/animationloader.h"
#include "Madgine/skeletonloader/skeletonloader.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT AnimationState : BehaviorReceiver {
            
            AnimationState(Render::AnimationLoader::Handle handle, IndexType<uint32_t> index);

            void start();
            void finish();
            
            void step(float delta);
            void setStep(float step);
            float currentStep() const;

            bool updateRender(std::chrono::microseconds frameTimeSinceLastFrame, std::chrono::microseconds sceneTimeSinceLastFrame, Matrix4 *matrices);

            Entity *entity();
            SceneManager *scene();

            struct stop_cb {
                stop_cb(AnimationState &state)
                    : mState(state)
                {
                }

                bool operator()();

                AnimationState &mState;
            };

            struct finally_cb {
                finally_cb(AnimationState &state)
                    : mState(state)
                {
                }

                void operator()(Execution::cancelled_t)
                {
                    mState.set_done();
                }
                void operator()()
                {
                    mState.set_value();
                }

                AnimationState &mState;
            };

            Execution::stop_callback<stop_cb, finally_cb> mStopCallback;

            Render::AnimationLoader::Handle mAnimationList;

            int *mBoneIndexMapping = nullptr;
            IndexType<uint32_t> mCurrentAnimation;
            float mCurrentStep = 0.0f;

        protected:
            template <typename>
            friend struct Execution::ConnectionStack;
            
            std::atomic<AnimationState *> mNext = nullptr;
        };

        Behavior animation(Render::AnimationLoader::Handle handle, Render::AnimationDescriptor *desc);
        Behavior animation(Render::AnimationLoader::Handle handle, std::string_view name);

    }
}
}

NATIVE_BEHAVIOR_DECLARATION(entity_animation)