#pragma once

namespace Engine {
namespace Debug {

    template <typename T>
    const T &min(const T &a, const T &b)
    {
        return std::min(a, b);
    }

    template <typename T>
    const T &max(const T &a, const T &b)
    {
        return std::max(a, b);
    }

    template <typename... Ty>
    struct Record {
        Record() = default;
        Record(Ty... data)
            : mData(std::forward<Ty>(data)...)
        {
        }

        void operator+=(const Record &other)
        {
            ((std::get<Ty>(mData) += std::get<Ty>(other.mData)), ...);
        }

        void operator-=(const Record &other)
        {
            ((std::get<Ty>(mData) -= std::get<Ty>(other.mData)), ...);
        }

        Record operator-(const Record &other) const
        {
            Record tmp = *this;
            tmp -= other;
            return tmp;
        }

        void operator/=(size_t s)
        {
            ((std::get<Ty>(mData) /= s), ...);
        }

        Record operator/(size_t s) const
        {
            Record tmp = *this;
            tmp /= s;
            return tmp;
        }

        std::tuple<Ty...> mData;
    };

    template <typename... Ty>
    Record<Ty...> min(const Record<Ty...> &a, const Record<Ty...> &b)
    {
        return { Debug::min(std::get<Ty>(a.mData), std::get<Ty>(b.mData))... };
    }

    template <typename... Ty>
    Record<Ty...> max(const Record<Ty...> &a, const Record<Ty...> &b)
    {
        return { Debug::max(std::get<Ty>(a.mData), std::get<Ty>(b.mData))... };
    }

    template <typename T>
    struct HistoryData {
        HistoryData()
            : mCurrentTotal {}
            , mMin { std::numeric_limits<T>::max() }
            , mMax { std::numeric_limits<T>::lowest() }
            , mTotal {}
            , mCount(0)
            , mIndex(0)
        {
        }

        T average(size_t size) const
        {
            return mCurrentTotal / min(max(size_t { 1 }, mCount), size);
        }

        T mCurrentTotal, mMin, mMax, mTotal;
        size_t mCount, mIndex;
    };

    template <typename T, size_t S>
    struct History {
        History()
            : mBuffer { {} }
        {
        }

        void operator<<(T value)
        {
            size_t currentIndex = mData.mIndex;
            mData.mIndex = (mData.mIndex + 1) % S;
            mData.mMin = min(mData.mMin, value);
            mData.mMax = max(mData.mMax, value);
            mData.mCurrentTotal += value - mBuffer[currentIndex];
            mBuffer[currentIndex] = value;
            mData.mTotal += value;
            ++mData.mCount;
        }

        const HistoryData<T> &data() const
        {
            return mData;
        }

        T average() const
        {
            return data().average(S);
        }

        const T *buffer() const
        {
            return mBuffer.data();
        }

        void resetExtremeValues()
        {
            mData.mMax = std::numeric_limits<float>::lowest();
            mData.mMin = std::numeric_limits<float>::max();
        }

    private:
        HistoryData<T> mData;
        std::array<T, S> mBuffer;
    };

    template <typename T, size_t S>
    struct Atomic_History {
        Atomic_History()
            : mBuffer { {} }
        {
        }

        void operator<<(T value)
        {
            HistoryData<T> data = mData.load();
            HistoryData<T> newData;
            size_t currentIndex;
            do {
                currentIndex = data.mIndex;
                newData.mIndex = (data.mIndex + 1) % S;
                newData.mMin = min(data.mMin, value);
                newData.mMax = max(data.mMax, value);
                newData.mCurrentTotal = data.mCurrentTotal + value - mBuffer[currentIndex];
                newData.mTotal = data.mTotal + value;
                newData.mCount = data.mCount + 1;
            } while (!mData.compare_exchange_strong(data, newData));
            mBuffer[currentIndex] = data;
        }

        HistoryData<T> data() const
        {
            return mData.load();
        }

        T average() const
        {
            return data().average(S);
        }

        const T *buffer() const
        {
            return mBuffer.data();
        }

    private:
        std::atomic<HistoryData<T>> mData;
        std::array<T, S> mBuffer;
    };

}
}