#include "../../scenelib.h"

#include "animation.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../entity/components/skeleton.h"

#include "Meta/math/transformation.h"

#include "../entity/entity.h"

#include "../entity/entitycomponentptr.h"

#include "transform.h"

namespace Engine {

namespace Scene {
    namespace Entity {

        template <typename Rec>
        struct AnimationStateImpl : VirtualBehaviorState<Rec, AnimationState> {

            virtual Entity *entity() override
            {
                Entity *entity;
                get_binding<"Entity">(this->mRec, entity);
                return entity;
            }
            virtual SceneManager *scene() override
            {
                SceneManager *scene;
                get_binding<"Scene">(this->mRec, scene);
                return scene;
            }

            using VirtualBehaviorState<Rec, AnimationState>::VirtualBehaviorState;
        };

        struct AnimationSender : Execution::base_sender {
            using result_type = GenericResult;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, AnimationSender &&sender, Rec &&rec)
            {
                return AnimationStateImpl<Rec> { std::forward<Rec>(rec), std::move(sender.mAnimation), sender.mCurrentAnimation };
            }

            template <typename F>
            friend void tag_invoke(Execution::visit_state_t, AnimationSender &sender, F &&f)
            {
                f(Execution::State::BeginBlock { "Play '"s + std::string { sender.mAnimation->mAnimations[sender.mCurrentAnimation].mName } + "'" });

                f(Execution::State::Contextual {
                    [](const void *context) -> Execution::StateDescriptor {
                        float progress = 0.0f;
                        if (context) {
                            const AnimationState *state = static_cast<const AnimationState *>(context);
                            float duration = state->mAnimationList->mAnimations[state->mCurrentAnimation].mDuration;
                            progress = fmodf(state->mCurrentStep, duration) / duration;
                        }
                        return Execution::State::Progress { progress };
                    } });

                f(Execution::State::EndBlock {});
            }

            static constexpr size_t debug_start_increment = 1;
            static constexpr size_t debug_operation_increment = 1;
            static constexpr size_t debug_stop_increment = 1;

            Render::AnimationLoader::Handle mAnimation;
            IndexType<uint32_t> mCurrentAnimation;
        };

        AnimationState::AnimationState(Render::AnimationLoader::Handle handle, IndexType<uint32_t> index)
            : mAnimationList(std::move(handle))
            , mCurrentAnimation(index)
            , mStopCallback(*this)
        {
        }

        void AnimationState::start()
        {
            mBoneIndexMapping = mAnimationList->generateBoneMappings(entity()->getComponent<Scene::Entity::Skeleton>()->get());

            scene()->addAnimation(this);
            mStopCallback.start(stopToken(), *this);
        }

        void AnimationState::finish()
        {
            mStopCallback.finish();
        }

        bool AnimationState::stop_cb::operator()()
        {
            return mState.scene()->stopAnimation(&mState);
        }

        void AnimationState::step(float delta)
        {
            mCurrentStep += delta;
        }

        void AnimationState::setStep(float step)
        {
            mCurrentStep = step;
        }

        float AnimationState::currentStep() const
        {
            return mCurrentStep;
        }

        bool AnimationState::updateRender(std::chrono::microseconds frameTimeSinceLastFrame, std::chrono::microseconds sceneTimeSinceLastFrame, Matrix4 *matrices)
        {
            mCurrentStep += std::chrono::duration_cast<std::chrono::duration<float>>(sceneTimeSinceLastFrame).count();
            Render::AnimationDescriptor &animation = mAnimationList->mAnimations[mCurrentAnimation];
            Skeleton *skeleton = entity()->getComponent<Skeleton>();

            float step = fmodf(mCurrentStep, animation.mDuration);
            if (step < 0.0f)
                step += animation.mDuration;

            std::set<size_t> parentTransformToDos;

            std::vector<Matrix4> localMatrices { skeleton->data()->mBones.size() };

            for (size_t i = 0; i < skeleton->data()->mBones.size(); ++i) {
                int mappedBone = mBoneIndexMapping ? mBoneIndexMapping[i] : i;
                if (mappedBone != -1) {
                    Render::AnimationBone &boneData = animation.mBones[mappedBone];

                    auto it_position_end = std::upper_bound(boneData.mPositions.begin(), boneData.mPositions.end(), step, [](float step, const Render::KeyFrame<Vector3> &k) { return step < k.mTime; });
                    assert(it_position_end != boneData.mPositions.begin());
                    auto it_position = std::prev(it_position_end);

                    Vector3 pos;
                    if (it_position_end != boneData.mPositions.end()) {
                        float position_blend = (step - it_position->mTime) / (it_position_end->mTime - it_position->mTime);
                        pos = lerp(it_position->mValue, it_position_end->mValue, position_blend);
                    } else {
                        pos = it_position->mValue;
                    }

                    auto it_scale_end = std::upper_bound(boneData.mScalings.begin(), boneData.mScalings.end(), step, [](float step, const Render::KeyFrame<Vector3> &k) { return step < k.mTime; });
                    assert(it_scale_end != boneData.mScalings.begin());
                    auto it_scale = std::prev(it_scale_end);

                    Vector3 scale;
                    if (it_scale_end != boneData.mScalings.end()) {
                        float scale_blend = (step - it_scale->mTime) / (it_scale_end->mTime - it_scale->mTime);
                        scale = lerp(it_scale->mValue, it_scale_end->mValue, scale_blend);
                    } else {
                        scale = it_scale->mValue;
                    }

                    auto it_orientation_end = std::upper_bound(boneData.mOrientations.begin(), boneData.mOrientations.end(), step, [](float step, const Render::KeyFrame<Quaternion> &k) { return step < k.mTime; });
                    assert(it_orientation_end != boneData.mOrientations.begin());
                    auto it_orientation = std::prev(it_orientation_end);

                    Quaternion orientation;
                    if (it_orientation_end != boneData.mOrientations.end()) {
                        float orientation_blend = (step - it_orientation->mTime) / (it_orientation_end->mTime - it_orientation->mTime);
                        orientation = slerp(it_orientation->mValue, it_orientation_end->mValue, orientation_blend);
                    } else {
                        orientation = it_orientation->mValue;
                    }

                    localMatrices[i] = skeleton->data()->mBones[i].mPreTransform * TransformMatrix(pos, scale, orientation);

                    IndexType<uint32_t> parent = skeleton->data()->mBones[i].mParent;
                    if (parent) {
                        if (parent < i && parentTransformToDos.count(parent) == 0)
                            matrices[i] = matrices[parent] * matrices[i];
                        else
                            parentTransformToDos.emplace(i);
                    }
                }
            }
            while (!parentTransformToDos.empty()) {
                for (std::set<size_t>::iterator it = parentTransformToDos.begin(); it != parentTransformToDos.end();) {
                    size_t parentIndex = skeleton->data()->mBones[*it].mParent;
                    if (parentTransformToDos.count(parentIndex) == 0) {
                        localMatrices[*it] = localMatrices[parentIndex] * localMatrices[*it];
                        it = parentTransformToDos.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            for (size_t i = 0; i < skeleton->data()->mBones.size(); ++i) {
                int mappedBone = mBoneIndexMapping ? mBoneIndexMapping[i] : i;
                if (mappedBone != -1)
                    matrices[i] = (skeleton->data()->mMatrix * localMatrices[i] * skeleton->data()->mMatrix1 * skeleton->data()->mBones[i].mOffsetMatrix * skeleton->data()->mMatrix2).Transpose();
            }

            return mCurrentStep > animation.mDuration;
        }

        Behavior animation(Render::AnimationLoader::Handle handle, Render::AnimationDescriptor *desc)
        {
            return AnimationSender { {}, std::move(handle), 0 };
        }

        Behavior animation(Render::AnimationLoader::Handle handle, std::string_view name)
        {
            auto it = std::ranges::find(handle->mAnimations, name, &Render::AnimationDescriptor::mName);
            return animation(std::move(handle), &*it);
        }

    }
}
}

NATIVE_BEHAVIOR(entity_animation, Engine::Scene::Entity::animation, Engine::InputParameter<Engine::Render::AnimationLoader::Handle>, Engine::InputParameter<std::string>)