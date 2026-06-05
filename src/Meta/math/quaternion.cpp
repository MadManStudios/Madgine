#include "../metalib.h"

#include "quaternion.h"

namespace Engine {
namespace Math {

    Quaternion slerp(Quaternion q1, Quaternion q2, float ratio)
    {

        // Compute the cosine of the angle between the two vectors.
        float dot = q1.dotProduct(q2);

        // If the dot product is negative, slerp won't take
        // the shorter path. Note that v1 and -v1 are equivalent when
        // the negation is applied to all four components. Fix by
        // reversing one quaternion.
        if (dot < 0.0f) {
            q1 = -q1;
            dot = -dot;
        }

        const float DOT_THRESHOLD = 0.9995f;
        if (dot > DOT_THRESHOLD) {
            // If the inputs are too close for comfort, linearly interpolate
            // and normalize the result.

            Quaternion result = q1 + (q2 - q1) * ratio;
            result.normalize();
            return result;
        }

        // Since dot is in range [0, DOT_THRESHOLD], acos is safe
        const float theta = acosf(dot);
        const float invsintheta = 1.0f / sinf(theta);
        const float scale = sinf(theta * (1.0f - ratio)) * invsintheta;
        const float invscale = sinf(theta * ratio) * invsintheta;
        Quaternion q = (q1 * scale) + (q2 * invscale);
        q.normalize();
        return q;
    }

}
}