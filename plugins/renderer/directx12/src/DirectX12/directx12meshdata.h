#pragma once

#include "util/directx12buffer.h"
#include "directx12textureloader.h"

#include "Madgine/meshloader/gpumeshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12MeshData : GPUMeshData {

        DirectX12Buffer mVertices;
        DirectX12Buffer mIndices;

        std::vector<TextureLoader::Handle> mTextureCache;
    };

}
}