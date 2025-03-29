#include "../metalib.h"

#include "geometry3.h"

#include "boundingbox.h"
#include "plane.h"
#include "ray3.h"
#include "sphere.h"
#include "vector3.h"

#include "common.h"

namespace Engine {

float Distance(const Ray3 &ray, const Vector3 &point, float *rayClosestParameter)
{
    if (rayClosestParameter)
        *rayClosestParameter = ray.mDir.dotProduct(point - ray.mPoint);
    return ray.mDir.crossProduct(point - ray.mPoint).length();
}

UpTo<float, 2> Intersect(const Ray3 &ray, const Sphere &sphere)
{
    Vector3 oc = ray.mPoint - sphere.mCenter;
    float a = 1.0f/* ray.mDir.dotProduct(ray.mDir) */;
    float b = 2.0f * oc.dotProduct(ray.mDir);
    float c = oc.dotProduct(oc) - sphere.mRadius * sphere.mRadius;
    float discriminant = b * b - 4 * a * c;

    UpTo<float, 2> result;
    if (discriminant >= 0) {
        result.push_back((-b - sqrt(discriminant)) / (2.0f * a));
        if (discriminant > 0)
            result.push_back((-b + sqrt(discriminant)) / (2.0f * a));
    }
    return result;
}

UpTo<float, 2> Intersect(const Ray3 &ray, const BoundingBox &box)
{
    Plane minX { box.mMin, box.mDirX };
    Plane maxX { box.mMax, box.mDirX };

    float tMin = std::numeric_limits<float>::lowest();
    float tMax = std::numeric_limits<float>::max();

    if (auto intersection = Intersect(ray, minX))
        tMin = intersection[0];
    if (auto intersection = Intersect(ray, maxX))
        tMax = intersection[0];

    if (tMin > tMax)
        std::swap(tMin, tMax);

    Plane minY { box.mMin, box.mDirY };
    Plane maxY { box.mMax, box.mDirY };

    float t2Min = std::numeric_limits<float>::lowest();
    float t2Max = std::numeric_limits<float>::max();

    if (auto intersection = Intersect(ray, minY))
        t2Min = intersection[0];
    if (auto intersection = Intersect(ray, maxY))
        t2Max = intersection[0];

    if (t2Min > t2Max)
        std::swap(t2Min, t2Max);

    tMin = std::max(tMin, t2Min);
    tMax = std::min(tMax, t2Max);

    if (tMin > tMax)
        return {};

    NormalizedVector3 dirZ = box.mDirX.crossProduct(box.mDirY);

    Plane minZ { box.mMin, dirZ };
    Plane maxZ { box.mMax, dirZ };

    t2Min = std::numeric_limits<float>::lowest();
    t2Max = std::numeric_limits<float>::max();

    if (auto intersection = Intersect(ray, minZ))
        t2Min = intersection[0];
    if (auto intersection = Intersect(ray, maxZ))
        t2Max = intersection[0];

    if (t2Min > t2Max)
        std::swap(t2Min, t2Max);

    tMin = std::max(tMin, t2Min);
    tMax = std::min(tMax, t2Max);

    UpTo<float, 2> result;

    if (tMax >= tMin) {
        result.push_back(tMin);
        if (tMax > tMin)
            result.push_back(tMax);
    }

    return result;
}

UpTo<float, 1> Intersect(const Ray3 &ray, const Plane &plane)
{
    UpTo<float, 1> result;

    float denom = plane.mNormal.dotProduct(ray.mDir);
    if (!isZero(denom)) {
        Vector3 pDist = plane.mNormal * plane.mDistance - ray.mPoint;
        float t = pDist.dotProduct(plane.mNormal) / denom;
        result.push_back(t);
    }
    return result;
}

}