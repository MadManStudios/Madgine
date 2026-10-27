#include "../renderlib.h"
#include "Madgine/imageloaderlib.h"

#include "rendermeshloader.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/meshloader/meshdata.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "rendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::RenderMeshLoader, Engine::Render::GPUMeshLoader);

namespace Engine {
namespace Render {

    struct Wrapper : GPUMeshData::ResourceBlockHolder {
        ~Wrapper()
        {
            if (mResourceBlock)
                RenderContext::getSingleton().destroyResourceBlock(mResourceBlock);
        }
    };

    RenderMeshLoader::RenderMeshLoader()
    {
        getOrCreateManual("quad", {}, {}, this);
        getOrCreateManual("Cube", {}, {}, this);
        getOrCreateManual("Plane", {}, {}, this);
    }

    Threading::Task<bool> RenderMeshLoader::generate(GPUMeshData &data, const MeshData &mesh)
    {
        data.mVertices = RenderContext::getSingleton().allocateBuffer<Void[]>(mesh.mVertices.mSize);

        co_await RenderContext::getSingleton().updateBuffer(data.mVertices, [&](auto &vertices) {
            std::memcpy(vertices, mesh.mVertices.begin(), mesh.mVertices.mSize);
        });

        if (!mesh.mIndices.empty()) {
            data.mIndices = RenderContext::getSingleton().allocateBuffer<uint32_t[]>(mesh.mIndices.size(), UsageHint::USAGE_INDEX);
            co_await RenderContext::getSingleton().updateBuffer(data.mIndices, [&](auto &indices) {
                std::ranges::copy(mesh.mIndices, indices);
            });
        }

        if (!co_await GPUMeshLoader::generate(data, mesh))
            co_return false;

        for (const MeshData::Material &mat : mesh.mMaterials) {
            GPUMeshData::Material &gpuMat = data.mMaterials.emplace_back();
            gpuMat.mName = mat.mName;
            gpuMat.mDiffuseColor = mat.mDiffuseColor;
            gpuMat.mEmissiveColor = mat.mEmissiveColor;

            std::vector<Threading::TaskFuture<bool>> futures;

            TexturePtr diffuseTexture;
            TexturePtr emissiveTexture;

            Resources::ImageLoader::Handle diffuseImage;
            Resources::ImageLoader::Handle emissiveImage;

            if (!mat.mDiffuseName.empty()) {
                futures.push_back(diffuseImage.load(mat.mDiffuseName));
            }

            if (!mat.mEmissiveName.empty()) {
                futures.push_back(emissiveImage.load(mat.mEmissiveName));
            }

            for (Threading::TaskFuture<bool> &fut : futures) {
                bool result = co_await fut;
                if (!result) {
                    LOG_ERROR("Missing Materials!");
                    co_return false;
                }
            }

            if (diffuseImage)
                diffuseTexture = RenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, diffuseImage->mSize, diffuseImage->mBuffer);
            if (emissiveImage)
                emissiveTexture = RenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, emissiveImage->mSize, emissiveImage->mBuffer);

            struct helper {
                float shininess;
            };
            GPUPtr<helper> buffer = RenderContext::getSingleton().allocateBuffer<helper>();

            co_await RenderContext::getSingleton().updateBuffer(buffer, [](auto &mapped) {
                mapped.shininess = 32.0f;
            });

            std::shared_ptr<Wrapper> wrapper = std::make_shared<Wrapper>();
            wrapper->mResourceBlock = RenderContext::getSingleton().createResourceBlock({ std::move(diffuseTexture), std::move(emissiveTexture), buffer });

            gpuMat.mResourceBlock = wrapper;
        }

        co_return true;
    }

    void RenderMeshLoader::reset(GPUMeshData &data)
    {
        data.mVertices = {};
        data.mIndices = {};
        for (GPUMeshData::Material &gpuMat : data.mMaterials) {
            gpuMat.mResourceBlock.reset();
        }
        GPUMeshLoader::reset(data);
    }

    Threading::TaskQueue *RenderMeshLoader::loadingTaskQueue() const
    {
        return RenderContext::renderQueue();
    }

}
}
