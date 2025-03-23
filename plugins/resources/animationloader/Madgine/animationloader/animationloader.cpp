#include "../animationloaderlib.h"

#include "animationloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

RESOURCELOADER(Engine::Render::AnimationLoader)

METATABLE_BEGIN(Engine::Render::AnimationList)
MEMBER(mAnimations)
METATABLE_END(Engine::Render::AnimationList)

METATABLE_BEGIN(Engine::Render::AnimationDescriptor)
MEMBER(mName)
MEMBER(mDuration)
MEMBER(mTicksPerSecond)
MEMBER(mBones)
METATABLE_END(Engine::Render::AnimationDescriptor)

METATABLE_BEGIN(Engine::Render::AnimationBone)
MEMBER(mPositions)
MEMBER(mOrientations)
MEMBER(mScalings)
METATABLE_END(Engine::Render::AnimationBone)

METATABLE_BEGIN(Engine::Render::VectorKeyFrame)
MEMBER(mTime)
MEMBER(mValue)
METATABLE_END(Engine::Render::VectorKeyFrame)

METATABLE_BEGIN(Engine::Render::QuaternionKeyFrame)
MEMBER(mTime)
MEMBER(mValue)
METATABLE_END(Engine::Render::QuaternionKeyFrame)

namespace Engine {
namespace Render {

    AnimationLoader::AnimationLoader()
        : ResourceLoader({ ".fbx", ".dae" })
    {
    }

    Threading::Task<bool> AnimationLoader::loadImpl(AnimationList &animations, ResourceDataInfo &info)
    {
        Assimp::Importer importer;

        ByteBuffer buffer = (co_await info.resource()->readAsync()).value();

        const aiScene *scene = importer.ReadFileFromMemory(buffer.mData, buffer.mSize, 0);

        if (!scene) {
            LOG_ERROR(importer.GetErrorString());
            co_return false;
        }

        for (size_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
            aiAnimation *anim = scene->mAnimations[animationIndex];

            AnimationDescriptor &animation = animations.mAnimations.emplace_back();

            animation.mName = anim->mName.C_Str();

            animation.mDuration = anim->mDuration;
            animation.mTicksPerSecond = anim->mTicksPerSecond;

            for (size_t boneIndex = 0; boneIndex < anim->mNumChannels; ++boneIndex) {
                aiNodeAnim *node = anim->mChannels[boneIndex];

                AnimationBone &bone = animation.mBones.emplace_back();

                if (animationIndex == 0) {
                    std::string_view name = node->mNodeName.C_Str();
                    if (StringUtil::endsWith(name, "_$AssimpFbx$_Rotation"))
                        name.remove_suffix(strlen("_$AssimpFbx$_Rotation"));
                    animations.mBoneNames.emplace_back(name);
                } else {
                    if (animations.mBoneNames.size() <= boneIndex) {
                        LOG_ERROR("Different amount of bones between animations in one set!");
                        animations.mBoneNames.emplace_back(node->mNodeName.C_Str());
                    } else {
                        if (animations.mBoneNames[boneIndex] != node->mNodeName.C_Str())
                            LOG_ERROR("Bone assignment mismatch between animations in one set! Will lead to broken animations!");
                    }
                }

                std::transform(node->mPositionKeys, node->mPositionKeys + node->mNumPositionKeys, std::back_inserter(bone.mPositions), [&](const aiVectorKey &key) {
                    return KeyFrame<Vector3> { static_cast<float>(key.mTime), Vector3 { &key.mValue.x } };
                });

                std::transform(node->mRotationKeys, node->mRotationKeys + node->mNumRotationKeys, std::back_inserter(bone.mOrientations), [&](const aiQuatKey &key) {
                    return KeyFrame<Quaternion> { static_cast<float>(key.mTime), Quaternion { key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w } };
                });

                std::transform(node->mScalingKeys, node->mScalingKeys + node->mNumScalingKeys, std::back_inserter(bone.mScalings), [&](const aiVectorKey &key) {
                    return KeyFrame<Vector3> { static_cast<float>(key.mTime), Vector3 { &key.mValue.x } };
                });
            }
        }

        co_return true;
    }

    Threading::Task<void> AnimationLoader::unloadImpl(AnimationList &data)
    {
        co_return;
    }

    int *AnimationList::generateBoneMappings(const SkeletonLoader::Handle &handle)
    {
        auto pib = mBoneMappings.try_emplace(handle);
        if (pib.second) {
            size_t i;
            bool consecutive = (mBoneNames.size() <= handle->mBones.size());
            if (!consecutive) {
                LOG_ERROR("Skeleton/Animation bone size mismatch!");
                pib.first->second = std::make_unique<int[]>(handle->mBones.size());
            }
            for (i = 0; i < handle->mBones.size(); ++i) {
                if (consecutive) {
                    if (mBoneNames[i] != handle->mBones[i].mName) {
                        consecutive = false;
                        pib.first->second = std::make_unique<int[]>(handle->mBones.size());
                        for (size_t j = 0; j < i; ++j) {
                            pib.first->second[j] = j;
                        }
                    }
                }
                if (!consecutive) {
                    auto it = std::ranges::find_if(mBoneNames, [&](const std::string &boneName) { return StringUtil::startsWith(handle->mBones[i].mName, boneName); });
                    int mapping;
                    if (it == mBoneNames.end()) {
                        mapping = -1;
                        LOG_WARNING("Animation targets non-existing Bone '" << handle->mBones[i].mName << "'!");
                    } else {
                        mapping = it - mBoneNames.begin();
                    }
                    pib.first->second[i] = mapping;
                }
            }
        }
        return pib.first->second.get();
    }

}
}