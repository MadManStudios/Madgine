#pragma once

namespace Engine {

template <typename T>
struct SafeIterator {
    using value_type = typename derive_iterator<T>::iterator::value_type;

    using traits = derive_iterator<std::vector<value_type>>;

    SafeIterator(T &&t)
        : mContainer(std::forward<T>(t))
        , mData(mContainer.begin(), mContainer.end())
    {
    }

    template <typename It>
    struct IteratorImpl {
        It mIt, mEnd;
        const std::vector<value_type> &mV;
        T &mRef;

        IteratorImpl(It it, It end, const std::vector<value_type> &v, T &ref)
            : mIt(std::move(it))
            , mEnd(std::move(end))
            , mV(v)
            , mRef(ref)
        {
            validate();
        }

        void validate()
        {
            while (mIt != mEnd && std::ranges::find(mRef, *mIt) == mRef.end())
                ++mIt;
        }

        value_type operator*() const
        {
            return *mIt;
        }

        bool operator!=(const IteratorImpl &other) const
        {
            return mIt != other.mIt;
        }

        IteratorImpl &operator++()
        {
            ++mIt;
            validate();
            return *this;
        }

        IteratorImpl operator++(int)
        {
            IteratorImpl copy = *this;
            ++*this;
            return copy;
        }
    };

    using iterator = IteratorImpl<typename traits::const_iterator>;
    using const_iterator = IteratorImpl<typename traits::const_iterator>;
    using reverse_iterator = IteratorImpl<typename traits::reverse_iterator>;
    using const_reverse_iterator = IteratorImpl<typename traits::const_reverse_iterator>;

    iterator begin()
    {
        return { mData.begin(), mData.end(), mData, mContainer };
    }

    iterator end()
    {
        return { mData.end(), mData.end(), mData, mContainer };
    };

    reverse_iterator rbegin()
    {
        return { mData.rbegin(), mData.rend(), mData, mContainer };
    }

    reverse_iterator rend()
    {
        return { mData.rend(), mData.rend(), mData, mContainer };
    };

    const_iterator begin() const
    {
        return { mData.begin(), mData.end(), mData, mContainer };
    }

    const_iterator end() const
    {
        return { mData.end(), mData.end(), mData, mContainer };
    };

    const_reverse_iterator rbegin() const
    {
        return { mData.rbegin(), mData.rend(), mData, mContainer };
    }

    const_reverse_iterator rend() const
    {
        return { mData.rend(), mData.rend(), mData, mContainer };
    };

private:
    T mContainer;
    std::vector<value_type> mData;
};

template <typename T>
SafeIterator<T> safeIterate(T &&t)
{
    return { std::forward<T>(t) };
}

}
