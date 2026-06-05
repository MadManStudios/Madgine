#pragma once

namespace Engine {
namespace Containers {

    template <typename T>
    struct SafeIterator {
        using value_type = std::ranges::range_value_t<T>;

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

        using iterator = IteratorImpl<std::ranges::iterator_t<T>>;
        using const_iterator = IteratorImpl<std::ranges::const_iterator_t<T>>;

        iterator begin()
        {
            return { mData.begin(), mData.end(), mData, mContainer };
        }

        iterator end()
        {
            return { mData.end(), mData.end(), mData, mContainer };
        };

        const_iterator begin() const
        {
            return { mData.begin(), mData.end(), mData, mContainer };
        }

        const_iterator end() const
        {
            return { mData.end(), mData.end(), mData, mContainer };
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
}