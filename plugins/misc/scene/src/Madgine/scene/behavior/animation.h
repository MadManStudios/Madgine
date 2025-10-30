#pragma once

#include "Madgine/animationloader/animationloader.h"
#include "Madgine/skeletonloader/skeletonloader.h"

#include "Generic/execution/stop_callback.h"

#include "Madgine/behaviorreceiver.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT AnimationState : BehaviorReceiver {
            
            AnimationState(Render::AnimationLoader::Handle handle, IndexType<uint32_t> index);

            void start();
            void stop();            
            
            void step(float delta);
            void setStep(float step);
            float currentStep() const;

            bool updateRender(std::chrono::microseconds frameTimeSinceLastFrame, std::chrono::microseconds sceneTimeSinceLastFrame, Matrix4 *matrices);

            Entity *entity();
            SceneManager *scene();

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