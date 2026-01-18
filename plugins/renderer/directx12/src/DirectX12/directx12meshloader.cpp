#include "Madgine/imageloaderlib.h"
#include "directx12lib.h"

#include "directx12meshloader.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/meshloader/meshdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "directx12meshdata.h"
#include "directx12rendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::DirectX12MeshLoader, Engine::Render::GPUMeshLoader);

namespace Engine {
namespace Render {

    DirectX12MeshLoader::DirectX12MeshLoader()
    {
        getOrCreateManual("quad", {}, {}, this);
        getOrCreateManual("Cube", {}, {}, this);
        getOrCreateManual("Plane", {}, {}, this);
    }

    Threading::Task<bool> DirectX12MeshLoader::generate(GPUMeshData &_data, const MeshData &mesh)
    {
        DirectX12MeshData &data = static_cast<DirectX12MeshData &>(_data);

        if (mesh.mVertices.mData)
            data.mVertices.setData(mesh.mVertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        if (!mesh.mIndices.empty())
            data.mIndices.setData(mesh.mIndices, D3D12_RESOURCE_STATE_INDEX_BUFFER);

        if (!co_await GPUMeshLoader::generate(_data, mesh))
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
                diffuseTexture = DirectX12RenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, diffuseImage->mSize, diffuseImage->mBuffer);
            if (emissiveImage)
                emissiveTexture = DirectX12RenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, emissiveImage->mSize, emissiveImage->mBuffer);

            struct helper {
                float shininess;
            };
            GPUPtr<helper> buffer = DirectX12RenderContext::getSingleton().allocateBuffer<helper>();

            {
                auto mapped = DirectX12RenderContext::getSingleton().mapBuffer(buffer);
                mapped->shininess = 32.0f;
            }

            gpuMat.mResourceBlock = DirectX12RenderContext::getSingleton().createResourceBlock({ std::move(diffuseTexture), std::move(emissiveTexture), buffer });
        }

        co_return true;
    }

    void DirectX12MeshLoader::reset(GPUMeshData &data)
    {
        static_cast<DirectX12MeshData &>(data).mVertices.reset();
        static_cast<DirectX12MeshData &>(data).mIndices.reset();
        static_cast<DirectX12MeshData &>(data).mTextureCache.clear();
        for (GPUMeshData::Material &gpuMat : data.mMaterials) {
            if (gpuMat.mResourceBlock)
                DirectX12RenderContext::getSingleton().destroyResourceBlock(gpuMat.mResourceBlock);
        }
        GPUMeshLoader::reset(data);
    }

    Threading::TaskQueue *DirectX12MeshLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }

}
}
