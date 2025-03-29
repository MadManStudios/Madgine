#include "../meshloaderlib.h"

#include "meshloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/render/vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../assimptools.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Interfaces/filesystem/fsapi.h"

RESOURCELOADER(Engine::Render::MeshLoader)

namespace Engine {
namespace Render {

    MeshLoader::MeshLoader()
        : ResourceLoader({ ".fbx", ".dae", ".stl" })
    {
        getOrCreateManual(
            "quad", {}, [](MeshLoader *loader, MeshData &data, MeshLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
                std::vector<Compound<VertexPos>> vertices {
                    { { -1, -1, 0 } },
                    { { 1, -1, 0 } },
                    { { -1, 1, 0 } },
                    { { 1, 1, 0 } }
                };

                std::vector<uint32_t> indices {
                    0, 1, 2, 1, 2, 3
                };

                data = { 3, std::move(vertices), std::move(indices) };
                co_return true;
            },
            this);

        getOrCreateManual(
            "Cube", {}, [](MeshLoader *loader, MeshData &data, MeshLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
                std::vector<Compound<VertexPos, VertexColor>> vertices {
                    { { -0.5f, -0.5f, -0.5f }, { 0, 0, 0, 1 } },
                    { { 0.5f, -0.5f, -0.5f }, { 1, 0, 0, 1 } },
                    { { -0.5f, 0.5f, -0.5f }, { 0, 1, 0, 1 } },
                    { { 0.5f, 0.5f, -0.5f }, { 1, 1, 0, 1 } },
                    { { -0.5f, -0.5f, 0.5f }, { 0, 0, 1, 1 } },
                    { { 0.5f, -0.5f, 0.5f }, { 1, 0, 1, 1 } },
                    { { -0.5f, 0.5f, 0.5f }, { 0, 1, 1, 1 } },
                    { { 0.5f, 0.5f, 0.5f }, { 1, 1, 1, 1 } }
                };

                std::vector<uint32_t> indices {
                    0, 1, 2, 1, 2, 3,
                    0, 1, 4, 1, 4, 5,
                    4, 5, 6, 5, 6, 7,
                    2, 3, 6, 3, 6, 7,
                    1, 3, 5, 3, 5, 7,
                    0, 2, 4, 2, 4, 6
                };

                data = { 3, std::move(vertices), std::move(indices) };
                co_return true;
            },
            this);

        getOrCreateManual(
            "Plane", {}, [](MeshLoader *loader, MeshData &data, MeshLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
                std::vector<Compound<VertexPos, VertexPosW>> vertices {
                    { { 0, 0, 0 }, 1 },
                    { { 1, 0, 0 }, 0 },
                    { { 0, 0, 1 }, 0 },
                    { { -1, 0, 0 }, 0 },
                    { { 0, 0, -1 }, 0 }
                };

                std::vector<uint32_t> indices {
                    0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1
                };

                data = { 3, std::move(vertices), std::move(indices) };
                co_return true;
            },
            this);
    }

    template <typename V>
    bool loadVertices(MeshData &mesh, MeshLoader &loader, const aiScene *scene, std::vector<MeshData::Material> materials)
    {
        std::vector<V> vertices;
        std::vector<uint32_t> indices;
        //Reserve??

        std::map<aiNode *, size_t> boneIndices;

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

            bool newBone = boneIndices.try_emplace(node, boneIndices.size()).second;
            assert(newBone);
        }

        bool warnOnce = true;

        assimpTraverseTree(
            scene, [&](const aiNode *node, const Matrix4 &t) {
                Matrix3 transform_anti = t.ToMat3().Inverse().Transpose();

                for (size_t meshIndex = 0; meshIndex < node->mNumMeshes; ++meshIndex) {
                    uint32_t baseVertexIndex = vertices.size();
                    aiMesh *mesh = scene->mMeshes[node->mMeshes[meshIndex]];

                    for (size_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
                        V &vertex = vertices.emplace_back();
                        aiVector3D v = mesh->mVertices[vertexIndex];
                        vertex.mPos = (t * Vector4 { v.x, v.y, v.z, 1.0f }).xyz();
                        if constexpr (V::template holds<VertexNormal>) {
                            aiVector3D n = mesh->mNormals[vertexIndex];
                            vertex.mNormal = transform_anti * Vector3 { n.x, n.y, n.z };
                        }
                        if constexpr (V::template holds<VertexColor>) {
                            if (mesh->mColors[0]) {
                                aiColor4D &c = mesh->mColors[0][vertexIndex];
                                vertex.mColor = Color4 { &c.r };
                            } else {
                                vertex.mColor = Vector4::UNIT_SCALE;
                            }
                        }
                        if constexpr (V::template holds<VertexUV>) {
                            if (mesh->mTextureCoords[0]) {
                                aiVector3D &uv = mesh->mTextureCoords[0][vertexIndex];
                                vertex.mUV = { uv.x, 1.0f - uv.y };
                            }
                        }
                    }

                    if constexpr (V::template holds<VertexBoneIndices>) {
                        static_assert(V::template holds<VertexBoneWeights>);
                        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                            aiBone *bone = mesh->mBones[boneIndex];
                            size_t actualIndex = boneIndices.at(scene->mRootNode->FindNode(bone->mName));
                            for (size_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                                size_t vertexIndex = baseVertexIndex + bone->mWeights[weightIndex].mVertexId;
                                float weight = bone->mWeights[weightIndex].mWeight;
                                float(&weights)[4] = vertices[vertexIndex].mBoneWeights;
                                int(&indices)[4] = vertices[vertexIndex].mBoneIndices;
                                size_t freeIndex = 0;
                                while (freeIndex < 4 && weights[freeIndex] != 0.0f)
                                    ++freeIndex;
                                if (freeIndex == 4) {
                                    if (warnOnce) {
                                        warnOnce = false;
                                        LOG_WARNING("Vertex is assigned more than 4 Bones. That is not supported currently, animation might look wrong.");
                                    }
                                } else {
                                    indices[freeIndex] = actualIndex;
                                    weights[freeIndex] = weight;
                                }
                            }
                        }
                    }

                    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
                        aiFace &face = mesh->mFaces[i];

                        uint32_t v0 = face.mIndices[0];
                        uint32_t lastIndex = face.mIndices[1];

                        for (size_t j = 2; j < face.mNumIndices; ++j) {
                            uint32_t index = face.mIndices[j];

                            indices.push_back(v0 + baseVertexIndex);
                            indices.push_back(lastIndex + baseVertexIndex);
                            indices.push_back(index + baseVertexIndex);
                            lastIndex = index;
                        }
                    }
                }
            });

        mesh = { 3, std::move(vertices), std::move(indices), std::move(materials) };
        return true;
    }

    Threading::Task<bool> MeshLoader::loadImpl(MeshData &mesh, ResourceDataInfo &info)
    {
        Assimp::Importer importer;

        auto result = co_await info.resource()->readAsync();

        if (result.is_done()) {
            LOG_ERROR("File read was cancelled");
            co_return false;
        }

        ByteBuffer buffer = std::move(result).value();

        const aiScene *scene = importer.ReadFileFromMemory(buffer.mData, buffer.mSize, aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_LimitBoneWeights);

        if (!scene) {
            LOG_ERROR(importer.GetErrorString());
            co_return false;
        }

        if (scene->mNumMeshes == 0) {
            LOG_ERROR("No mesh in file '" << info.resource()->path().str() << "'");
            co_return false;
        }

        std::vector<MeshData::Material> materials;
        for (size_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
            aiMaterial *mat = scene->mMaterials[materialIndex];
            if (mat->GetName() == aiString { "DefaultMaterial" })
                continue; //TODO: Find proper solution

            MeshData::Material &targetMat = materials.emplace_back();
            targetMat.mName = mat->GetName().C_Str();
            aiColor3D aiDiffuseColor;
            if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuseColor)) {
                targetMat.mDiffuseColor = { aiDiffuseColor.r, aiDiffuseColor.g, aiDiffuseColor.b, 1.0f };
            }
            size_t count = mat->GetTextureCount(aiTextureType_DIFFUSE);
            for (size_t i = 0; i < count; ++i) {
                aiString diffuseTexturePath;
                if (mat->GetTexture(aiTextureType_DIFFUSE, i, &diffuseTexturePath) == AI_SUCCESS && diffuseTexturePath.length > 0) {
                    std::string_view diffuseName { diffuseTexturePath.C_Str() };
                    auto it = std::ranges::find_if(diffuseName | std::views::reverse, Filesystem::isSeparator).base();
                    diffuseName = diffuseName.substr(std::distance(diffuseName.begin(), it));
                    if (!targetMat.mDiffuseName.empty() && targetMat.mDiffuseName != diffuseName)
                        throw "Only one diffuse texture is allowed at the moment!";
                    targetMat.mDiffuseName = Filesystem::Path { diffuseName }.stem();
                }
            }

            count = mat->GetTextureCount(aiTextureType_EMISSIVE);
            for (size_t i = 0; i < count; ++i) {
                aiString emissiveTexturePath;
                if (mat->GetTexture(aiTextureType_EMISSIVE, i, &emissiveTexturePath) == AI_SUCCESS && emissiveTexturePath.length > 0) {
                    std::string_view emissiveName { emissiveTexturePath.C_Str() };
                    auto it = std::ranges::find_if(emissiveName | std::views::reverse, Filesystem::isSeparator).base();
                    emissiveName = emissiveName.substr(std::distance(emissiveName.begin(), it));
                    if (!targetMat.mEmissiveName.empty() && targetMat.mEmissiveName != emissiveName)
                        throw "Only one emissive texture is allowed at the moment!";
                    targetMat.mEmissiveName = Filesystem::Path { emissiveName }.stem();
                }
            }
        }

        bool hasSkeleton = false;
        assimpTraverseTree(scene, [&](const aiNode *node) {
            for (size_t i = 0; i < node->mNumMeshes; ++i) {
                hasSkeleton |= scene->mMeshes[node->mMeshes[i]]->HasBones();
            }
        });

        if (!hasSkeleton) {
            if (materials.empty()) {
                co_return loadVertices<Render::Vertex>(mesh, *this, scene, std::move(materials));
            } else {
                co_return loadVertices<Render::Vertex3>(mesh, *this, scene, std::move(materials));
            }
        } else {
            if (materials.empty()) {
                co_return loadVertices<Render::Vertex4>(mesh, *this, scene, std::move(materials));
            } else {
                co_return loadVertices<Render::Vertex5>(mesh, *this, scene, std::move(materials));
            }
        }
    }

    Threading::Task<void> MeshLoader::unloadImpl(MeshData &data)
    {
        data.mGroupSize = 0;
        data.mIndices.clear();
        data.mMaterials.clear();
        data.mVertexSize = 0;
        data.mVertices.clear();
        co_return;
    }

}
}