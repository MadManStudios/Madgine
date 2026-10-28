#pragma once

#include <array>
#include <cmath>

#include "Meta/math/boundingbox.h"
#include "Meta/math/frustum.h"
#include "Meta/math/matrix4.h"
#include "Meta/math/vector3.h"
#include "Meta/math/vector4.h"

namespace Engine {
namespace Render {

    // Culling against the planes of a frustum (perspective or orthographic) in world space: an object whose world space
    // bounding box lies completely outside one of them cannot touch the image. Conservative, never drops a visible
    // object; boxes that only pass a corner of the frustum are kept. The near plane is left out: the pipelines render
    // without depth clipping (DirectX12: DepthClipEnable = FALSE), so casters between the sun and the near plane of a
    // shadow map still throw their shadow (clamped to depth 0). Behind a perspective camera the side planes cull.
    struct MADGINE_SCENE_RENDERER_EXPORT FrustumCuller {
        // Off: everything is drawn as before (for comparisons)
        static inline bool sEnabled = true;

        FrustumCuller(const Math::Frustum &frustum)
        {
            std::array<Math::Vector3, 8> corners = frustum.getCorners();
            // far, bottom, right, top, left (corner order of Frustum::getCorners)
            static constexpr int faces[5][3] = { { 4, 5, 6 }, { 0, 1, 5 }, { 1, 2, 6 }, { 2, 3, 7 }, { 3, 0, 4 } };
            Math::Vector3 centre { Math::Vector3::ZERO };
            for (const Math::Vector3 &c : corners)
                centre += c;
            centre /= 8.0f;
            for (size_t i = 0; i < 5; ++i) {
                const Math::Vector3 &o = corners[faces[i][0]];
                Math::Vector3 n = (corners[faces[i][1]] - o).crossProduct(corners[faces[i][2]] - o);
                if (n.dotProduct(centre - o) < 0.0f)
                    n = -n;
                mPlanes[i] = { n, -n.dotProduct(o) };
            }
        }

        bool visible(const Math::AABB &aabb, const Math::Matrix4 &transform) const
        {
            // No vertices (or no box): keep it
            if (!sEnabled || !(aabb.mMin.x <= aabb.mMax.x && aabb.mMin.y <= aabb.mMax.y && aabb.mMin.z <= aabb.mMax.z))
                return true;
            Math::Vector3 centre = (transform * Math::Vector4 { (aabb.mMin + aabb.mMax) * 0.5f, 1.0f }).xyz();
            Math::Vector3 halfSize = (aabb.mMax - aabb.mMin) * 0.5f;
            // Extent of the transformed box along the world axes: |M| * halfSize
            Math::Vector3 extent;
            for (size_t axis = 0; axis < 3; ++axis)
                extent[axis] = std::abs(transform[axis][0]) * halfSize.x + std::abs(transform[axis][1]) * halfSize.y + std::abs(transform[axis][2]) * halfSize.z;
            for (const Math::Vector4 &p : mPlanes) {
                float radius = std::abs(p.x) * extent.x + std::abs(p.y) * extent.y + std::abs(p.z) * extent.z;
                if (p.x * centre.x + p.y * centre.y + p.z * centre.z + p.w < -radius)
                    return false;
            }
            return true;
        }

        std::array<Math::Vector4, 5> mPlanes;
    };

}
}
