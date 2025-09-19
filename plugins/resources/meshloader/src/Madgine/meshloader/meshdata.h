#pragma once

#include "Generic/bytebuffer.h"
#include "Generic/offsetptr.h"
#include "Meta/math/boundingbox.h"
#include "Madgine/render/vertex.h"
#include "Madgine/render/vertexformat.h"

namespace Engine {
namespace Render {

    struct MeshData {

        struct Material {
            std::string mName;
            std::string mDiffuseName;
            std::string mEmissiveName;
            Vector4 mDiffuseColor = Vector4::UNIT_SCALE;
        };

        template <typename VertexType>
        static AABB calculateAABB(const std::vector<VertexType> &vertices)
        {
            Vector3 minP { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
            Vector3 maxP { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

            for (const VertexType &v : vertices) {
                const Vector3 &pos = v.mPos.xyz();
                minP = min(pos, minP);
                maxP = max(pos, maxP);
            }
            return { minP,
                maxP };
        }

        
        MeshData() = default;

        template <typename VertexType>
        MeshData(size_t groupSize, std::vector<VertexType> vertices, std::vector<uint32_t> indices = {}, std::vector<Material> materials = {})
            : mFormat(type_holder<VertexType>)
            , mAABB(calculateAABB(vertices))
            , mGroupSize(groupSize)
            , mVertices(std::move(vertices))
            , mVertexSize(sizeof(VertexType))
            , mIndices(std::move(indices))
            , mMaterials(std::move(materials))
        {
        }

        VertexFormat mFormat;
        AABB mAABB;
        size_t mGroupSize;
        ByteBuffer mVertices;
        size_t mVertexSize;
        std::vector<uint32_t> mIndices;
        std::vector<Material> mMaterials;
    };

}
}