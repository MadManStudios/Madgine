#pragma once

namespace Engine {

template <typename... T>
struct ConcatItContainer {
    using iterator_traits = derive_iterator<first_t<T...>>;
    using value_type = typename std::remove_reference_t<first_t<T...>>::value_type;
    using reference = typename std::remove_reference_t<first_t<T...>>::reference;

    template <typename... It>
    struct ConcatIterator {
        using iterator_category = typename first_t<It...>::iterator_category;
        using value_type = typename first_t<It...>::value_type;
        using difference_type = ptrdiff_t;
        using pointer = void;
        using reference = typename first_t<It...>::reference;

        ConcatIterator() = default;

        ConcatIterator(std::tuple<std::pair<It, It>...> it)
            : mIt(std::move(it))
        {
            update();
        }

        ConcatIterator(const ConcatIterator<It...> &other) = default;
        ConcatIterator(ConcatIterator<It...> &&other) = default;
        ConcatIterator &operator=(const ConcatIterator<It...> &other)
        {
            mIt = other.mIt;
            return *this;
        }
        ConcatIterator &operator=(ConcatIterator<It...> &&other)
        {
            mIt = std::move(other.mIt);
            return *this;
        }

        ConcatIterator &operator++()
        {
            assert(mIndex < sizeof...(It));
            TupleUnpacker::select(
                mIt, [](auto &it) -> decltype(auto) {
                    ++it.first;
                },
                mIndex);
            update();
            return *this;
        }

        ConcatIterator operator++(int)
        {
            ConcatIterator copy = *this;
            ++copy;
            return copy;
        }

        decltype(auto) operator*() const
        {
            return TupleUnpacker::select(
                mIt, [](auto &it) -> decltype(auto) {
                    return *it.first;
                },
                mIndex);
        }

        bool operator!=(const ConcatIterator<It...> &other) const
        {
            return mIt != other.mIt;
        }

        bool operator==(const ConcatIterator<It...> &other) const
        {
            return mIt == other.mIt;
        }

    private:
        void update()
        {
            while (mIndex < sizeof...(It) && TupleUnpacker::select(
                       mIt, [](auto &it) -> decltype(auto) {
                           return it.first == it.second;
                       },
                       mIndex))
                ++mIndex;
        }

    private:
        std::tuple<std::pair<It, It>...> mIt;
        size_t mIndex = 0;
    };

    using iterator = ConcatIterator<typename derive_iterator<T>::iterator...>;
    using const_iterator = ConcatIterator<typename derive_iterator<T>::const_iterator...>;
    using reverse_iterator = ConcatIterator<typename derive_iterator<T>::reverse_iterator...>;
    using const_reverse_iterator = ConcatIterator<typename derive_iterator<T>::const_reverse_iterator...>;

private:
    ConcatItContainer() = delete;
    ConcatItContainer(const ConcatItContainer &) = delete;

    ConcatItContainer &operator=(const ConcatItContainer &) = delete;

public:
    ConcatItContainer(ConcatItContainer &&) = default;

    ConcatItContainer &operator=(ConcatItContainer &&) = default;

    ConcatItContainer(T &&...containers)
        : mContainers(std::forward<T>(containers)...)
    {
    }

    iterator begin()
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.begin(), c.end()); }) };
    }

    const_iterator begin() const
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.begin(), c.end()); }) };
    }

    iterator end()
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.end(), c.end()); }) };
    }

    const_iterator end() const
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.end(), c.end()); }) };
    }

    reverse_iterator rbegin()
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.rbegin(), c.rend()); }) };
    }

    const_reverse_iterator rbegin() const
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.rbegin(), c.rend()); }) };
    }

    reverse_iterator rend()
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.rend(), c.rend()); }) };
    }

    const_reverse_iterator rend() const
    {
        return { TupleUnpacker::forEach(mContainers, [](auto &c) { return std::make_pair(c.rend(), c.rend()); }) };
    }

    bool empty() const
    {
        return TupleUnpacker::accumulate(
            mContainers, [](auto &c, bool b) { return c.empty() && b; }, true);
    }

    size_t size() const
    {
        return TupleUnpacker::accumulate(
            mContainers, [](auto &c, size_t s) { return c.size() + s; }, 0);
    }

    bool isReference() const
    {
        constexpr auto check = [](auto &&c) {
            if constexpr (std::ranges::borrowed_range<decltype(c)>) {
                return true;
            } else if constexpr (has_function_isReference_v<decltype(c)>) {
                return c.isReference();
            } else {
                return false;
            }
        };
        bool first = check(std::get<0>(mContainers));
        TupleUnpacker::forEach(std::move(mContainers), [=](auto &&c) { assert(check(c) == first); });
        return first;
    }

private:
    std::tuple<std::views::all_t<T>...> mContainers;
};

template <typename... T>
ConcatItContainer<T...> concatIt(T &&...t)
{
    return { std::forward<T>(t)... };
}

}

template <typename... T>
inline constexpr bool std::ranges::enable_borrowed_range<Engine::ConcatItContainer<T...>> = (std::ranges::borrowed_range<T> && ...);