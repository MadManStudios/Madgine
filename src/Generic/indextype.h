#pragma once

namespace Engine {

template <Unsigned T, T invalid = std::numeric_limits<T>::max()>
struct IndexType {

    using value_type = T;
    static constexpr T sInvalid = invalid;

    constexpr IndexType(T index = invalid)
        : mIndex(index)
    {
    }

    constexpr IndexType(const IndexType &) = default;

    constexpr IndexType(IndexType &&other)
        : mIndex(std::exchange(other.mIndex, invalid))
    {
    }

    template <std::convertible_to<T> U>
    constexpr IndexType(IndexType<U> other)
        : mIndex(other)
    {
    }

    constexpr IndexType &operator=(T t)
    {
        assert(t != invalid);
        mIndex = t;
        return *this;
    }

    constexpr IndexType &operator=(const IndexType &) = default;

    constexpr IndexType &operator=(IndexType &&other)
    {
        mIndex = std::exchange(other.mIndex, invalid);
        return *this;
    }

    constexpr void operator++()
    {
        assert(mIndex != invalid);
        ++mIndex;
    }
    constexpr void operator--()
    {
        assert(mIndex != invalid);
        --mIndex;
    }
    constexpr IndexType &operator+=(T s)
    {
        assert(mIndex != invalid);
        mIndex += s;
        return *this;
    }
    constexpr IndexType &operator-=(T s)
    {
        assert(mIndex != invalid);
        mIndex -= s;
        return *this;
    }

    constexpr bool operator==(const IndexType &other) const
    {
        return mIndex == other.mIndex;
    }

    template <std::constructible_from<T> U>
    constexpr bool operator==(const U &other) const
    {
        return mIndex == other;
    }
        
    template <std::constructible_from<T> U>
    friend constexpr bool operator==(const U &other, const IndexType &self)
    {
        return self.mIndex == other;
    }

    template <std::constructible_from<T> U>
    constexpr bool operator==(const IndexType<U> &other) const
    {
        return mIndex == other.mIndex;
    }

    template <std::constructible_from<T> U>
    constexpr bool operator!=(const U &other) const
    {
        return mIndex != other;
    }

    constexpr void reset()
    {
        mIndex = invalid;
    }

    constexpr operator T() const
    {
        return mIndex;
    }

    explicit constexpr operator bool() const
    {
        return mIndex != invalid;
    }

    friend std::ostream &operator<<(std::ostream &out, const IndexType &val)
    {
        int64_t rep = val ? static_cast<int64_t>(val) : -1;
        return out << rep;
    }

    friend std::istream &operator>>(std::istream &in, IndexType &val)
    {
        int64_t rep;
        if (in >> rep) {
            if (rep == -1)
                val.reset();
            else
                val = rep;
        }
        return in;
    }

    T mIndex;
};

}
/*
namespace std {
template <typename T, typename U, template <typename> class TQual, template <typename> class UQual>
struct basic_common_reference<Engine::IndexType<T>, U, TQual, UQual> {
    using type = std::common_reference_t<T, U>;
};
template <typename T, typename U, template <typename> class TQual, template <typename> class UQual>
struct basic_common_reference<T, Engine::IndexType<U>, TQual, UQual> {
    using type = std::common_reference_t<T, U>;
};
}*/