#include "../moduleslib.h"

#include "customclock.h"

namespace Engine {
namespace Threading {

    CustomTimepoint::CustomTimepoint(std::chrono::steady_clock::time_point timepoint, const CustomClock *clock)
        : mTimePoint(timepoint)
        , mClock(clock)
    {
    }

    CustomTimepoint::CustomTimepoint(std::chrono::steady_clock::time_point timepoint)
        : mTimePoint(timepoint)
    {
    }

    CustomTimepoint::CustomTimepoint(std::chrono::steady_clock::duration dur)
        : mTimePoint(std::chrono::steady_clock::now() + dur)
    {
    }

    std::chrono::steady_clock::time_point CustomTimepoint::revert()
    {
        return mClock ? mClock->revert(mTimePoint) : mTimePoint;
    }

    std::chrono::steady_clock::duration CustomTimepoint::operator-(const CustomTimepoint &other) const
    {
        assert(mClock == other.mClock);
        return mTimePoint - other.mTimePoint;
    }

    CustomTimepoint CustomTimepoint::operator+(const std::chrono::steady_clock::duration other) const
    {
        return {
            mTimePoint + other,
            mClock
        };
    }

    std::strong_ordering CustomTimepoint::operator<=>(const std::chrono::steady_clock::time_point other) const
    {
        return mTimePoint <=> (mClock ? mClock->get(other) : other);
    }

    std::strong_ordering CustomTimepoint::operator<=>(const CustomTimepoint &other) const
    {
        assert(mClock == other.mClock);
        return mTimePoint <=> other.mTimePoint;
    }

}
}