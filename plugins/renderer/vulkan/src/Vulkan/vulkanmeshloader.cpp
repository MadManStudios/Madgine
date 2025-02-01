#include "vulkanlib.h"

#include "vulkanmeshloader.h"

#include "vulkanmeshdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "vulkanrendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::VulkanMeshLoader, Engine::Render::GPUMeshLoader);

namespace Engine {
namespace Render {

    VulkanMeshLoader::VulkanMeshLoader()
    {
        getOrCreateManual("quad", {}, {}, this);
        getOrCreateManual("Cube", {}, {}, this);
        getOrCreateManual("Plane", {}, {}, this);
    }

    Threading::Task<bool> VulkanMeshLoader::generate(GPUMeshData &_data, const MeshData &mesh)
    {
        VulkanMeshData &data = static_cast<VulkanMeshData &>(_data);

        data.mVertices.setData(mesh.mVertices);

        if (!mesh.mIndices.empty())
            data.mIndices.setData(mesh.mIndices);

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

            gpuMat.mResourceBlock = VulkanRenderContext::getSingleton().createResourceBlock({ &*diffuseTexture, &*emissiveTexture });
        }

        co_return true;
    }

    void VulkanMeshLoader::reset(GPUMeshData &data)
    {
        static_cast<VulkanMeshData &>(data).mVertices.reset();
        static_cast<VulkanMeshData &>(data).mIndices.reset();
        static_cast<VulkanMeshData &>(data).mTextureCache.clear();
        for (GPUMeshData::Material &gpuMat : data.mMaterials) {
            if (gpuMat.mResourceBlock)
                VulkanRenderContext::getSingleton().destroyResourceBlock(gpuMat.mResourceBlock);
        }        
        GPUMeshLoader::reset(data);
    }

    Threading::TaskQueue *VulkanMeshLoader::loadingTaskQueue() const
    {
        return VulkanRenderContext::renderQueue();
    }

}
}
