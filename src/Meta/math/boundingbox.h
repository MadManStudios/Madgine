#pragma once

#include "matrix4.h"

namespace Engine {
namespace Math {

    struct AABB {
        Vector3 mMin, mMax;

        constexpr float diameter() const
        {
            return (mMax - mMin).length();
        }

        constexpr Vector3 center() const
        {
            return 0.5f * (mMax + mMin);
        }

        std::array<Vector3, 8> corners() const
        {
            std::array<Vector3, 8> result;
            result[0] = mMin;
            result[1] = { mMax.x, mMin.y, mMin.z };
            result[2] = { mMin.x, mMax.y, mMin.z };
            result[3] = { mMax.x, mMax.y, mMin.z };
            result[4] = { mMin.x, mMin.y, mMax.z };
            result[5] = { mMax.x, mMin.y, mMax.z };
            result[6] = { mMin.x, mMax.y, mMax.z };
            result[7] = mMax;
            return result;
        }
    };

    struct BoundingBox {
        Vector3 mMin, mMax;
        NormalizedVector3 mDirX, mDirY;

        constexpr BoundingBox(const AABB &bb)
            : mMin(bb.mMin)
            , mMax(bb.mMax)
            , mDirX { { 1, 0, 0 } }
            , mDirY { { 0, 1, 0 } }
        {
        }

        constexpr BoundingBox(const Vector3 &min, const Vector3 &max, const NormalizedVector3 &dirX, const NormalizedVector3 &dirY)
            : mMin(min)
            , mMax(max)
            , mDirX(dirX)
            , mDirY(dirY)
        {
        }

        std::array<Vector3, 8> corners() const
        {
            Vector3 dist = mMax - mMin;
            NormalizedVector3 dirZ = mDirX.crossProduct(mDirY);
            std::array<Vector3, 8> result;
            result[0] = mMin;
            result[1] = mMin + dist.dotProduct(mDirX) * mDirX;
            result[2] = mMin + dist.dotProduct(mDirY) * mDirY;
            result[3] = mMax - dist.dotProduct(dirZ) * dirZ;
            result[4] = mMin + dist.dotProduct(dirZ) * dirZ;
            result[5] = mMax - dist.dotProduct(mDirY) * mDirY;
            result[6] = mMax - dist.dotProduct(mDirX) * mDirX;
            result[7] = mMax;
            return result;
        }
    };

    constexpr BoundingBox operator*(const Matrix4 &transform, const BoundingBox &bb)
    {
        return {
            (transform * Vector4 { bb.mMin, 1.0f }).xyz(),
            (transform * Vector4 { bb.mMax, 1.0f }).xyz(),
            (transform * Vector4 { bb.mDirX, 0.0f }).xyz(),
            (transform * Vector4 { bb.mDirY, 0.0f }).xyz()
        };
    }

}
}