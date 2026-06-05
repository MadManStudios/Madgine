#pragma once

#include "upto.h"

namespace Engine {
namespace Math {

    META_EXPORT float Distance(const Ray3 &ray, const Vector3 &point, float *rayClosestParameter = nullptr);

    META_EXPORT UpTo<float, 2> Intersect(const Ray3 &ray, const Sphere &sphere);
    META_EXPORT UpTo<float, 2> Intersect(const Ray3 &ray, const BoundingBox &box);
    META_EXPORT UpTo<float, 1> Intersect(const Ray3 &ray, const Plane &plane);
    META_EXPORT UpTo<float, 2> Intersect(const Ray3 &ray, const Frustum &frustum);
    META_EXPORT UpTo<float, 2> Intersect(const Ray3 &ray, const Plane &minX, const Plane &maxX, const Plane &minY, const Plane &maxY, const Plane &minZ, const Plane &maxZ);

}
}