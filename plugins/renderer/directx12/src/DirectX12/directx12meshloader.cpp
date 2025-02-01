#include "directx12lib.h"

#include "directx12meshloader.h"

#include "directx12meshdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/imageloaderlib.h"
#include "Madgine/meshloader/meshdata.h"

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

            TextureLoader::Handle &diffuseTexture = data.mTextureCache.emplace_back();
            futures.push_back(diffuseTexture.loadFromImage(mat.mDiffuseName.empty() ? "blank_black" : mat.mDiffuseName, TextureType_2D, FORMAT_RGBA8_SRGB));
            TextureLoader::Handle &emissiveTexture = data.mTextureCache.emplace_back();
            futures.push_back(emissiveTexture.loadFromImage(mat.mEmissiveName.empty() ? "blank_black" : mat.mEmissiveName, TextureType_2D, FORMAT_RGBA8_SRGB));

            for (Threading::TaskFuture<bool> &fut : futures) {
                bool result = co_await fut;
                if (!result) {
                    LOG_ERROR("Missing Materials!");
                    co_return false;
                }
            }

            gpuMat.mResourceBlock = DirectX12RenderContext::getSingleton().createResourceBlock({ &*diffuseTexture, &*emissiveTexture });
        }

        co_return true;
    }

    void DirectX12MeshLoader::reset(GPUMeshData &data)
    {
        static_cast<DirectX12MeshData&>(data).mVertices.reset();
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
