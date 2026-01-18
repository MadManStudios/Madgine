#pragma once

#include "Madgine/meshloader/gpumeshdata.h"

#include "util/directx12buffer.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12MeshData : GPUMeshData {

        DirectX12Buffer mVertices;
        DirectX12Buffer mIndices;

        std::vector<TexturePtr> mTextureCache;
    };

}
}