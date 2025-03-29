#pragma once

#include "Generic/compound.h"
#include "Meta/math/color4.h"

namespace Engine {
namespace Render {

    struct VertexPos {
        using ctor = Vector3;
        Vector3 mPos;
    };

    struct VertexPosW {
        using ctor = float;
        float mW;
    };

    struct VertexPos2 {
        using ctor = Vector2;
        Vector2 mPos2;
    };

    struct VertexNormal {
        using ctor = Vector3;
        Vector3 mNormal;
    };

    struct VertexColor {
        using ctor = Color4;
        Vector4 mColor;
    };

    struct VertexUV {
        using ctor = Vector2;
        Vector2 mUV;
    };

    struct VertexBoneIndices {
        using ctor = int[4];
        int mBoneIndices[4];
    };

    struct VertexBoneWeights {
        using ctor = float[4];
        float mBoneWeights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    using VertexElements = type_pack<
        VertexPos,
        VertexPosW,
        VertexPos2,
        VertexNormal,
        VertexColor,
        VertexUV,
        VertexBoneIndices,
        VertexBoneWeights>;

    static constexpr size_t sVertexElementSizes[] = {
        sizeof(VertexPos),
        sizeof(VertexPosW),
        sizeof(VertexPos2),
        sizeof(VertexNormal),
        sizeof(VertexColor),
        sizeof(VertexUV),
        sizeof(VertexBoneIndices),
        sizeof(VertexBoneWeights)
    };

    using Vertex = Compound<
        VertexPos,
        VertexNormal,
        VertexColor>;

    using Vertex2 = Compound<
        VertexPos,
        VertexPos2,
        VertexColor,
        VertexUV>;

    using Vertex3 = Compound<
        VertexPos,
        VertexNormal,
        VertexUV>;

    using Vertex4 = Compound<
        VertexPos,
        VertexNormal,
        VertexColor,
        VertexBoneIndices,
        VertexBoneWeights>;

    using Vertex5 = Compound<
        VertexPos,
        VertexNormal,
        VertexUV,        
        VertexBoneIndices,
        VertexBoneWeights>;

}
}