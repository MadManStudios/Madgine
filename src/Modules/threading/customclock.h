#pragma once

namespace Engine {
namespace Threading {
    struct MODULES_EXPORT CustomTimepoint {

        CustomTimepoint() = default;
        CustomTimepoint(std::chrono::steady_clock::time_point timepoint);
        CustomTimepoint(std::chrono::steady_clock::duration dur);

        std::chrono::steady_clock::time_point revert();

        std::strong_ordering operator<=>(const std::chrono::steady_clock::time_point other) const;
        std::strong_ordering operator<=>(const CustomTimepoint &other) const;

        std::chrono::steady_clock::duration operator-(const CustomTimepoint &other) const;
        CustomTimepoint operator+(const std::chrono::steady_clock::duration other) const;

    private:
        friend struct CustomClock;

        CustomTimepoint(std::chrono::steady_clock::time_point timepoint, const CustomClock *clock);

        std::chrono::steady_clock::time_point mTimePoint;
        const CustomClock *mClock = nullptr;
    };

    struct CustomClock {
        CustomTimepoint operator()(std::chrono::steady_clock::duration dur) const
        {
            return {
                get(std::chrono::steady_clock::now()) + dur,
                this
            };
        }

        CustomTimepoint now() const
        {
            return {
                get(std::chrono::steady_clock::now()),
                this
            };
        }

        virtual std::chrono::steady_clock::time_point get(std::chrono::steady_clock::time_point timepoint) const = 0;
        virtual std::chrono::steady_clock::time_point revert(std::chrono::steady_clock::time_point timepoint) const = 0;
    };

}
}