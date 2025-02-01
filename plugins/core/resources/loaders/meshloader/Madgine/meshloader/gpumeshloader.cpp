#include "../meshloaderlib.h"

#include "gpumeshloader.h"

#include "meshloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/threading/taskqueue.h"

VIRTUALRESOURCELOADERBASE(Engine::Render::GPUMeshLoader)

METATABLE_BEGIN(Engine::Render::GPUMeshData)
MEMBER(mMaterials)
METATABLE_END(Engine::Render::GPUMeshData)

METATABLE_BEGIN(Engine::Render::GPUMeshData::Material)
READONLY_PROPERTY(Name, mName)
/* MEMBER(mDiffuseTexture)
MEMBER(mEmissiveTexture)*/
MEMBER(mDiffuseColor)
METATABLE_END(Engine::Render::GPUMeshData::Material)

namespace Engine {
namespace Render {

    GPUMeshLoader::GPUMeshLoader()
        : VirtualResourceLoaderBase({ ".fbx", ".dae", ".stl" })
    {
    }

    Threading::Task<bool> GPUMeshLoader::loadImpl(GPUMeshData &mesh, ResourceDataInfo &info)
    {
        MeshLoader::Handle handle;
        if (!co_await handle.load(info.resource()->name()))
            co_return false;
        if (!co_await generate(mesh, *handle)) {
            LOG_ERROR("Failed to upload mesh to GPU: '" << info.resource()->name() << "'");
            co_return false;
        }
        co_return true;
    }

    void GPUMeshLoader::unloadImpl(GPUMeshData &data)
    {
        reset(data);
    }

    void GPUMeshLoader::reset(GPUMeshData &data)
    {
        data.mMaterials.clear();
    }

    Threading::Task<bool> GPUMeshLoader::generate(GPUMeshData &data, const MeshData &mesh)
    {
        GPUMeshLoader::reset(data);

        data.mFormat = mesh.mFormat;
        data.mVertexSize = mesh.mVertexSize;
        data.mAABB = mesh.mAABB;

        data.mGroupSize = mesh.mGroupSize;

        if (mesh.mIndices.empty()) {
            data.mElementCount = mesh.mVertices.mSize / mesh.mVertexSize;
        } else {
            data.mElementCount = mesh.mIndices.size();
        }

        co_return true;
    }

}
}