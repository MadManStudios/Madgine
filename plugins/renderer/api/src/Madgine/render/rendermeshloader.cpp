#include "../renderlib.h"
#include "Madgine/imageloaderlib.h"

#include "rendermeshloader.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/meshloader/meshdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "rendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::RenderMeshLoader, Engine::Render::GPUMeshLoader);

namespace Engine {
namespace Render {

    RenderMeshLoader::RenderMeshLoader()
    {
        getOrCreateManual("quad", {}, {}, this);
        getOrCreateManual("Cube", {}, {}, this);
        getOrCreateManual("Plane", {}, {}, this);
    }

    Threading::Task<bool> RenderMeshLoader::generate(GPUMeshData &data, const MeshData &mesh)
    {
        data.mVertices = RenderContext::getSingleton().allocateBuffer<Void[]>(mesh.mVertices.mSize);
        {
            auto vertices = RenderContext::getSingleton().mapBuffer(data.mVertices);
            std::memcpy(vertices.begin(), mesh.mVertices.begin(), mesh.mVertices.mSize);
        }

        if (!mesh.mIndices.empty()) {
            data.mIndices = RenderContext::getSingleton().allocateBuffer<uint32_t[]>(mesh.mIndices.size(), UsageHint::USAGE_INDEX);
            auto indices = RenderContext::getSingleton().mapBuffer(data.mIndices);
            std::ranges::copy(mesh.mIndices, indices.begin());
        }

        if (!co_await GPUMeshLoader::generate(data, mesh))
            co_return false;

        for (const MeshData::Material &mat : mesh.mMaterials) {
            GPUMeshData::Material &gpuMat = data.mMaterials.emplace_back();
            gpuMat.mName = mat.mName;
            gpuMat.mDiffuseColor = mat.mDiffuseColor;

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

            {
                auto mapped = RenderContext::getSingleton().mapBuffer(buffer);
                mapped->shininess = 32.0f;
            }

            gpuMat.mResourceBlock = RenderContext::getSingleton().createResourceBlock({ std::move(diffuseTexture), std::move(emissiveTexture), buffer });
        }

        co_return true;
    }

    void RenderMeshLoader::reset(GPUMeshData &data)
    {
        data.mVertices = {};
        data.mIndices = {};
        for (GPUMeshData::Material &gpuMat : data.mMaterials) {
            if (gpuMat.mResourceBlock)
                RenderContext::getSingleton().destroyResourceBlock(gpuMat.mResourceBlock);
        }
        GPUMeshLoader::reset(data);
    }

    Threading::TaskQueue *RenderMeshLoader::loadingTaskQueue() const
    {
        return RenderContext::renderQueue();
    }

}
}
