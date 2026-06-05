#pragma once

#include "Meta/math/boundingbox.h"

#include "Madgine/render/resourceblock.h"
#include "Madgine/render/vertexformat.h"

namespace Engine {
namespace Render {

    struct GPUMeshData {
        struct Material {
            std::string mName;
            UniqueResourceBlock mResourceBlock;
            Math::Vector4 mDiffuseColor = Math::Vector4::UNIT_SCALE;
        };
        std::vector<Material> mMaterials;
        Math::AABB mAABB;
        VertexFormat mFormat;
        size_t mGroupSize;
        size_t mElementCount;

        GPUPtr<Void[]> mVertices;
        GPUPtr<uint32_t[]> mIndices;
    };

}
}