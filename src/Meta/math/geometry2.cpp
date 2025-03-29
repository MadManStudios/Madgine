#include "../metalib.h"

#include "geometry2.h"

#include "line2.h"
#include "rect2.h"

#include "common.h"

namespace Engine {
META_EXPORT UpTo<float, 2> Intersect(const Line2 &line, const Rect2 &rect)
{
    UpTo<float, 2> rayResult = Intersect(line.toRay(), rect);
    UpTo<float, 2> result;
    if (rayResult) {
        if (rayResult[0] >= 0.0f && rayResult[0] <= line.length()) {
            result.push_back(rayResult[0] / line.length());
        }
        if (rayResult.size() > 1 && rayResult[1] >= 0.0f && rayResult[1] <= line.length()) {
            result.push_back(rayResult[1] / line.length());
        }
    }
    return result;
}

META_EXPORT UpTo<float, 2> Intersect(const Ray2 &ray, const Rect2 &rect)
{
    float tMin = std::numeric_limits<float>::lowest();
    float tMax = std::numeric_limits<float>::max();

    bool hit = false;
    if (auto intersection = Intersect(ray, rect.leftLine().toRay())) {
        hit = true;
        tMin = intersection[0].first;
    }
    if (auto intersection = Intersect(ray, rect.rightLine().toRay())) {
        hit = true;
        tMax = intersection[0].first;
    }
    if (!hit) {
        if (ray.mPoint.x < rect.left() || ray.mPoint.x > rect.right())
            return {};
    }

    if (tMin > tMax)
        std::swap(tMin, tMax);

    float t2Min = std::numeric_limits<float>::lowest();
    float t2Max = std::numeric_limits<float>::max();

    hit = false;
    if (auto intersection = Intersect(ray, rect.topLine().toRay())) {
        hit = true;
        t2Min = intersection[0].first;
    }
    if (auto intersection = Intersect(ray, rect.bottomLine().toRay())) {
        hit = true;
        t2Max = intersection[0].first;
    }
    if (!hit) {
        if (ray.mPoint.y < rect.top() || ray.mPoint.y > rect.bottom())
            return {};
    }

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

META_EXPORT UpTo<std::pair<float, float>, 1> Intersect(const Line2 &lineA, const Line2 &lineB)
{
    UpTo<std::pair<float, float>, 1> rayResult = Intersect(lineA.toRay(), lineB.toRay());
    UpTo<std::pair<float, float>, 1> result;
    if (rayResult) {
        float factorA = rayResult[0].first / lineA.length();
        float factorB = rayResult[0].second / lineB.length();
        if (factorA >= 0.0f && factorA <= 1.0f
            && factorB >= 0.0f && factorB <= 1.0f)
            result.push_back({ factorA, factorB });
    }
    return result;
}

META_EXPORT UpTo<std::pair<float, float>, 1> Intersect(const Ray2 &rayA, const Ray2 &rayB)
{
    UpTo<std::pair<float, float>, 1> result;

    float dx = rayB.mPoint.x - rayA.mPoint.x;
    float dy = rayB.mPoint.y - rayA.mPoint.y;
    float det = rayB.mDir.x * rayA.mDir.y - rayB.mDir.y * rayA.mDir.x;
    if (!isZero(det)) {
        float u = (dy * rayB.mDir.x - dx * rayB.mDir.y) / det;
        float v = (dy * rayA.mDir.x - dx * rayA.mDir.y) / det;
        result.push_back({ u, v });
    }

    return result;
}
}