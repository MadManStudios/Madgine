#pragma once

#include "upto.h"

namespace Engine {

META_EXPORT UpTo<float, 2> Intersect(const Line2 &line, const Rect2 &rect);
META_EXPORT UpTo<float, 2> Intersect(const Ray2 &line, const Rect2 &rect);
META_EXPORT UpTo<std::pair<float, float>, 1> Intersect(const Line2 &lineA, const Line2 &lineB);
META_EXPORT UpTo<std::pair<float, float>, 1> Intersect(const Ray2 &rayA, const Ray2 &rayB);


}