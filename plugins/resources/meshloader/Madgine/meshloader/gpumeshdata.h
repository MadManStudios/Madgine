#pragma once

#include "Meta/math/boundingbox.h"
#include "Madgine/render/vertexformat.h"
#include "Madgine/render/resourceblock.h"

namespace Engine {
namespace Render {

    struct GPUMeshData {
        struct Material {
            std::string mName;
            UniqueResourceBlock mResourceBlock;
            Vector4 mDiffuseColor = Vector4::UNIT_SCALE;
        };
        std::vector<Material> mMaterials;
        AABB mAABB;
        size_t mVertexSize;
        VertexFormat mFormat;
        size_t mGroupSize;
        size_t mElementCount;
    };

}
}