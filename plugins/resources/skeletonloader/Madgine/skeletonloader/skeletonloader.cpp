#include "../skeletonloaderlib.h"

#include "skeletonloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Meta/math/transformation.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../assimptools.h"

RESOURCELOADER(Engine::Render::SkeletonLoader)

METATABLE_BEGIN(Engine::Render::Bone)
MEMBER(mName)
MEMBER(mOffsetMatrix)
MEMBER(mTTransform)
//MEMBER(mParent)
//MEMBER(mFirstChild)
METATABLE_END(Engine::Render::Bone)

METATABLE_BEGIN(Engine::Render::SkeletonDescriptor)
MEMBER(mBones)
MEMBER(mMatrix)
MEMBER(mMatrix1)
MEMBER(mMatrix2)
METATABLE_END(Engine::Render::SkeletonDescriptor)

namespace Engine {
namespace Render {

    SkeletonLoader::SkeletonLoader()
        : ResourceLoader({ ".fbx", ".dae" })
    {
    }

    Threading::Task<bool> SkeletonLoader::loadImpl(SkeletonDescriptor &skeleton, ResourceDataInfo &info)
    {
        Assimp::Importer importer;

        ByteBuffer buffer = (co_await info.resource()->readAsync()).value();

        const aiScene *scene = importer.ReadFileFromMemory(buffer.mData, buffer.mSize, aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_OptimizeGraph);

        if (!scene) {
            LOG_ERROR(importer.GetErrorString());
            co_return false;
        }

        if (scene->mNumMeshes == 0) {
            LOG_ERROR("No mesh in file '" << info.resource()->path().str() << "'");
            co_return false;
        }

        std::map<aiNode *, size_t> indices;

        auto cmp = [](aiBone *first, aiBone *second) {
            if (first == second)
                return false;

            if (first->mName == second->mName) {
                assert(first->mOffsetMatrix == second->mOffsetMatrix);
                return false;
            }
            return strcmp(first->mName.C_Str(), second->mName.C_Str()) < 0;
        };
        std::set<aiBone *, decltype(cmp)> bones { cmp };

        for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
            aiMesh *mesh = scene->mMeshes[meshIndex];

            for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                bones.insert(mesh->mBones[boneIndex]);
            }
        }

        for (aiBone *bone : bones) {
            aiNode *node = scene->mRootNode->FindNode(bone->mName);

            bool newBone = indices.try_emplace(node, skeleton.mBones.size()).second;
            assert(newBone);
            skeleton.mBones.push_back(Bone { bone->mName.C_Str() });
            skeleton.mBones.back().mTTransform = assimpConvertMatrix(node->mTransformation);
            skeleton.mBones.back().mPreTransform = assimpConvertMatrix(node->mTransformation);
            skeleton.mBones.back().mPreTransform = Matrix4 { ExtractScalingMatrix(assimpConvertMatrix(node->mTransformation).ToMat3()) };
        }

        for (aiBone *bone : bones) {
            aiNode *node = scene->mRootNode->FindNode(bone->mName);

            size_t index = indices.at(node);
            Bone &boneData = skeleton.mBones[index];

            aiNode *current = node;
            while (current->mParent) {
                current = current->mParent;
                auto it = indices.find(current);
                if (it != indices.end()) {
                    boneData.mParent = it->second;
                    if (!skeleton.mBones[it->second].mFirstChild)
                        skeleton.mBones[it->second].mFirstChild = index;
                    break;
                } else {
                    boneData.mTTransform = assimpConvertMatrix(current->mTransformation) * boneData.mTTransform;
                    boneData.mPreTransform = assimpConvertMatrix(current->mTransformation) * boneData.mPreTransform;
                }
            }
        }

        bool matrixSet = false;
        skeleton.mMatrix = Matrix4::IDENTITY;

        std::set<size_t> parentTransformToDos;

        /* assimpTraverseTree(
            scene, [&](const aiNode *parentNode, const Matrix4 &t) {
                Matrix4 anti_t = t.Inverse();

                for (size_t meshIndex = 0; meshIndex < parentNode->mNumMeshes; ++meshIndex) {

                    aiMesh *mesh = scene->mMeshes[parentNode->mMeshes[meshIndex]];

                    if (mesh->mNumBones > 0) {

                        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                            aiBone *bone = mesh->mBones[boneIndex];*/
        for (aiBone *bone : bones) {
            aiNode *node = scene->mRootNode->FindNode(bone->mName);

            /* if (!matrixSet) {
                                matrixSet = true;
                                skeleton.mMatrix = t;
                            }*/

            size_t index = indices.at(node);
            Bone &boneData = skeleton.mBones[index];

            boneData.mOffsetMatrix = assimpConvertMatrix(bone->mOffsetMatrix) /* * anti_t*/;

            IndexType<uint32_t> parentIndex = boneData.mParent;
            if (parentIndex) {
                if (parentIndex < index && parentTransformToDos.count(parentIndex) == 0)
                    boneData.mTTransform = skeleton.mBones[parentIndex].mTTransform * boneData.mTTransform;
                else
                    parentTransformToDos.emplace(index);
            }
        }
        /*}
                }
            });*/

        while (!parentTransformToDos.empty()) {
            for (std::set<size_t>::iterator it = parentTransformToDos.begin(); it != parentTransformToDos.end();) {
                size_t parentIndex = skeleton.mBones[*it].mParent;
                if (parentTransformToDos.count(parentIndex) == 0) {
                    skeleton.mBones[*it].mTTransform = skeleton.mBones[parentIndex].mTTransform * skeleton.mBones[*it].mTTransform;
                    it = parentTransformToDos.erase(it);
                } else {
                    ++it;
                }
            }
        }

        for (Bone &bone : skeleton.mBones) {
            bone.mTTransform = skeleton.mMatrix * bone.mTTransform * bone.mOffsetMatrix;
        }

        co_return true;
    }

    Threading::Task<void> SkeletonLoader::unloadImpl(SkeletonDescriptor &data)
    {
        data.mBones.clear();
        co_return;
    }

}
}