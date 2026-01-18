#include "opengllib.h"

#include "openglmeshloader.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/meshloader/meshdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "openglmeshdata.h"
#include "openglrendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::OpenGLMeshLoader, Engine::Render::GPUMeshLoader);

namespace Engine {
namespace Render {

    OpenGLMeshLoader::OpenGLMeshLoader()
    {
        getOrCreateManual("quad", {}, {}, this);
        getOrCreateManual("Cube", {}, {}, this);
        getOrCreateManual("Plane", {}, {}, this);
    }

    Threading::Task<bool> OpenGLMeshLoader::generate(GPUMeshData &_data, const MeshData &mesh)
    {
        OpenGLMeshData &data = static_cast<OpenGLMeshData &>(_data);

        data.mVertices.setData(mesh.mVertices);

        if (!mesh.mIndices.empty())
            data.mIndices.setData(mesh.mIndices);

        if (!co_await GPUMeshLoader::generate(data, mesh))
            co_return false;

        for (const MeshData::Material &mat : mesh.mMaterials) {
            GPUMeshData::Material &gpuMat = data.mMaterials.emplace_back();
            gpuMat.mName = mat.mName;
            gpuMat.mDiffuseColor = mat.mDiffuseColor;

            std::vector<Threading::TaskFuture<bool>> futures;

            Resources::ImageLoader::Handle diffuseImage;
            futures.push_back(diffuseImage.load(mat.mDiffuseName.empty() ? "blank_black" : mat.mDiffuseName));
            Resources::ImageLoader::Handle emissiveImage;
            futures.push_back(emissiveImage.load(mat.mEmissiveName.empty() ? "blank_black" : mat.mEmissiveName));

            for (Threading::TaskFuture<bool> &fut : futures) {
                bool result = co_await fut;
                if (!result) {
                    LOG_ERROR("Missing Materials!");
                    co_return false;
                }
            }

            TexturePtr diffuseTexture = OpenGLRenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, diffuseImage->mSize, diffuseImage->mBuffer);
            TexturePtr emissiveTexture = OpenGLRenderContext::getSingleton().createTexture(TextureType_2D, TextureFormat::FORMAT_RGBA8_SRGB, emissiveImage->mSize, emissiveImage->mBuffer);

            gpuMat.mResourceBlock = OpenGLRenderContext::getSingleton().createResourceBlock({ std::move(diffuseTexture), std::move(emissiveTexture) });
        }

        co_return true;
    }

    void OpenGLMeshLoader::reset(GPUMeshData &data)
    {
        static_cast<OpenGLMeshData &>(data).mVertices.reset();
        static_cast<OpenGLMeshData &>(data).mIndices.reset();
        static_cast<OpenGLMeshData &>(data).mTextureCache.clear();
        for (GPUMeshData::Material &gpuMat : data.mMaterials) {
            if (gpuMat.mResourceBlock)
                OpenGLRenderContext::getSingleton().destroyResourceBlock(gpuMat.mResourceBlock);
        }
        GPUMeshLoader::reset(data);
    }

    Threading::TaskQueue *OpenGLMeshLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

}
}
