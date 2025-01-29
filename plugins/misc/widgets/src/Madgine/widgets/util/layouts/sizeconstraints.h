#pragma once

namespace Engine {
namespace Widgets {

    struct SizeConstraints {
        float mMin = 0.0f;
        float mPreferred = 0.0f;
        float mMax = 0.0f;

        SizeConstraints operator*(int n) const
        {
            return {
                mMin * n,
                mPreferred * n,
                mMax * n
            };
        }

        SizeConstraints operator+(float f) const
        {
            return {
                mMin + f,
                mPreferred + f,
                mMax + f
            };
        }

        SizeConstraints &operator+=(const SizeConstraints &other)
        {
            mMin += other.mMin;
            mPreferred += other.mPreferred;
            mMax += other.mMax;
            return *this;
        }

        float calculateFactor(float size) const
        {
            float diff = size - mPreferred;
            float range = (diff < 0.0f ? mPreferred - mMin : mMax - mPreferred);
            return std::clamp(diff / range, -1.0f, 1.0f);
        }

        float apply(float factor) const
        {
            float range = (factor < 0.0f ? mPreferred - mMin : mMax - mPreferred);
            return mPreferred + factor * range;
        }

        friend SizeConstraints max(const SizeConstraints &first, const SizeConstraints &second)
        {
            return {
                std::max(first.mMin, second.mMin),
                std::max(first.mPreferred, second.mPreferred),
                std::max(first.mMax, second.mMax)
            };
        }
    };

    struct SizeConstraints2 {
        SizeConstraints mWidth;
        SizeConstraints mHeight;
    };

}
}