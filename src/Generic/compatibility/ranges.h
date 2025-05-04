#pragma once

#include "../pipable.h"


#ifndef MADGINE_COMPATIBILITY_HAS_RANGES
#define MADGINE_COMPATIBILITY_HAS_RANGES __cpp_lib_ranges >= 201911L
#endif

#ifndef MADGINE_COMPATIBILITY_NEED_CONST_ITERATOR
#define MADGINE_COMPATIBILITY_NEED_CONST_ITERATOR __cpp_lib_ranges < 202207L || EMSCRIPTEN || OSX
#endif

#ifndef MADGINE_COMPATIBILITY_NEED_RANGES_CONCEPT
#define MADGINE_COMPATIBILITY_NEED_RANGES_CONCEPT __cpp_lib_ranges < 201911L
#endif

#ifndef MADGINE_COMPATIBILITY_NEED_RANGES_BORROWED_RANGE
#define MADGINE_COMPATIBILITY_NEED_RANGES_BORROWED_RANGE __cpp_lib_ranges < 202110L
#endif

#ifndef MADGINE_COMPATIBILITY_NEED_RANGES_RANGE_VALUE
#define MADGINE_COMPATIBILITY_NEED_RANGES_RANGE_VALUE __cpp_lib_ranges < 201911L
#endif


#if MADGINE_COMPATIBILITY_HAS_RANGES
#    include <ranges>
#endif

namespace std {
namespace ranges {

#if MADGINE_COMPATIBILITY_NEED_RANGES_CONCEPT
    template <class T>
    concept range = requires(T &t)
    {
        t.begin();
        t.end();
    };
#endif

#if __cpp_lib_ranges < 201911L
    struct equal_to{
        template <typename T1, typename T2>
        constexpr bool operator()(T1 &&left, T2 &&right) const{
            return std::forward<T1>(left) == std::forward<T2>(right);
        }
    };
#endif 

#if MADGINE_COMPATIBILITY_NEED_RANGES_RANGE_VALUE
    template <typename T>
    using range_value_t = typename std::remove_reference_t<T>::value_type;
#endif

#if __cpp_lib_ranges < 202110L
    template <typename C, typename F>
    constexpr auto find_if_not(C &&c, F &&f)
    {
        return std::find_if_not(std::forward<C>(c).begin(), std::forward<C>(c).end(), std::forward<F>(f));
    }

    template <typename C, typename F>
    constexpr auto find_if(C &&c, F &&f)
    {
        auto end = c.end();
        auto it = c.begin();
        for (; it != end; ++it) {
            if (f(*it)) {
                return it;
            }
        }
        return it;
    }

    template <typename I, typename V, typename P>
    constexpr auto find(I &&begin, I &&end, V &&v, P &&p)
    {
        auto it = std::move(begin);
        for (; it != end; ++it) {
            if (v == std::invoke(p, *it)) {
                return it;
            }
        }
        return it;
    }

    template <typename C, typename F, typename P>
    constexpr auto find_if(C &&c, F &&f, P &&p)
    {
        auto end = c.end();
        auto it = c.begin();
        for (; it != end; ++it) {
            if (f(std::invoke(p, *it))) {
                return it;
            }
        }
        return it;
    }

    template <typename C, typename O, typename F>
    constexpr auto transform(C &&c, O o, F &&op)
    {
        for (auto &&e : std::forward<C>(c))
            *o++ = op(std::forward<decltype(e)>(e));
        return o;
    }

    template <typename C, typename O>
    constexpr auto copy(C &&c, O o)
    {
        return std::copy(std::forward<C>(c).begin(), std::forward<C>(c).end(), o);
    }

    template <typename C, typename O>
    constexpr auto move(C &&c, O o)
    {
        return std::move(std::forward<C>(c).begin(), std::forward<C>(c).end(), o);
    }

    template<std::input_iterator I, std::sentinel_for<I> S, typename T,
             class Proj = std::identity>
    constexpr I find(I first, S last, const T& value, Proj proj = {})
    {
        for (; first != last; ++first)
            if (std::invoke(proj, *first) == value)
                return first;
        return first;
    }
 
    template<typename R, class T, class Proj = std::identity>
    constexpr auto
        find(R&& r, const T& value, Proj proj = {})
    {
        return find(ranges::begin(r), ranges::end(r), value, std::ref(proj));
    }

    template <typename C, typename Cmp>
    constexpr auto sort(C &&c, Cmp &&cmp)
    {
        return std::sort(std::forward<C>(c).begin(), std::forward<C>(c).end(), std::forward<Cmp>(cmp));
    }

    template <typename C, typename T>
    constexpr auto lower_bound(C &&c, T &&t)
    {
        return std::lower_bound(std::forward<C>(c).begin(), std::forward<C>(c).end(), std::forward<T>(t));
    }

    template<std::input_or_output_iterator I, std::sentinel_for<I> S>
    constexpr void advance(I& i, S bound)
    {
        if constexpr (std::assignable_from<I&, S>)
            i = std::move(bound);
        else if constexpr (std::sized_sentinel_for<S, I>)
            advance(i, bound - i);
        else
            while (i != bound)
                ++i;
    }

    template<std::input_or_output_iterator I, std::sentinel_for<I> S>
    constexpr std::iter_difference_t<I>
    advance(I& i, std::iter_difference_t<I> n, S bound)
    {
        if constexpr (std::sized_sentinel_for<S, I>)
        {
            // std::abs is not constexpr until C++23
            auto abs = [](const std::iter_difference_t<I> x) { return x < 0 ? -x : x; };
 
            if (const auto dist = abs(n) - abs(bound - i); dist < 0)
            {
                advance(i, bound);
                return -dist;
            }
 
            std::advance(i, n);
            return 0;
        }
        else
        {
            while (n > 0 && i != bound)
            {
                --n;
                ++i;
            }
 
            if constexpr (std::bidirectional_iterator<I>)
            {
                while (n < 0 && i != bound)
                {
                    ++n;
                    --i;
                }
            }
 
            return n;
        }
    }

    template<typename I, class T, std::sentinel_for<I> S,
             class Proj = std::identity, 
             typename Comp = std::less<>>
    constexpr I upper_bound(I first, S last, const T& value,
                           Comp comp = {}, Proj proj = {})
    {
        I it;
        std::iter_difference_t<I> count, step;
        count = std::distance(first, last);
 
        while (count > 0)
        {
            it = first; 
            step = count / 2;
            ranges::advance(it, step, last);
            if (!comp(value, std::invoke(proj, *it)))
            {
                first = ++it;
                count -= step + 1;
            }
            else
                count = step;
        }
        return first;
    }
 
    template<typename R, class T, class Proj = std::identity,
             typename Comp = std::less<>>
    constexpr ranges::borrowed_iterator_t<R>
        upper_bound(R&& r, const T& value, Comp comp = {}, Proj proj = {})
    {
        return upper_bound(ranges::begin(r), ranges::end(r), value,
                       std::ref(comp), std::ref(proj));
    }

    template <typename C, typename T>
    constexpr auto search_n(C &&c, size_t count, T &&t)
    {
        auto it = std::search_n(std::forward<C>(c).begin(), std::forward<C>(c).end(), count, std::forward<T>(t));
        struct subrange{

            auto begin() const{
                return mBegin;
            }

            auto end() const{
                return mEnd;
            }

            decltype(it) mBegin, mEnd;
        };

        
        return subrange{it, std::next(it, count)};
    }

    template <typename C, typename F>
    constexpr bool all_of(C &&c, F &&f)
    {
        return std::all_of(std::forward<C>(c).begin(), std::forward<C>(c).end(),
            std::forward<F>(f));
    }

#endif

#if __cpp_lib_ranges_contains < 202207L

    struct __contains_fn {
        template <std::input_iterator I, std::sentinel_for<I> S,
            class T, class Proj = std::identity>
        requires std::indirect_binary_predicate < ranges::equal_to, std::projected<I, Proj>,
        const T * > constexpr bool operator()(I first, S last, const T &value, Proj proj = {}) const
        {
            return ranges::find(std::move(first), last, value, proj) != last;
        }

        template <ranges::input_range R, class T, class Proj = std::identity>
        requires std::indirect_binary_predicate < ranges::equal_to,
            std::projected<ranges::iterator_t<R>, Proj>,
        const T * > constexpr bool operator()(R &&r, const T &value, Proj proj = {}) const
        {
            return (*this)(ranges::begin(r), ranges::end(r), std::move(value), proj);
        }
    };

    inline constexpr __contains_fn contains {};
#endif

#if MADGINE_COMPATIBILITY_NEED_CONST_ITERATOR
    template <range R>
    using const_iterator_t = decltype(cbegin(std::declval<R &>()));
#endif
}

#if MADGINE_COMPATIBILITY_NEED_RANGES_BORROWED_RANGE

namespace ranges {
    template <typename T>
    inline constexpr bool enable_borrowed_range = false;

    template <typename R>
    concept borrowed_range = range<R> &&(std::is_lvalue_reference_v<R> || enable_borrowed_range<std::remove_cvref_t<R>>);
}
#endif

#if __cpp_lib_ranges < 202110L

namespace ranges{
namespace views {
    template <typename R>
    struct all_t {

        all_t(R &&r)
            : mR(std::forward<R>(r))
        {
        }

        using iterator = decltype(std::declval<R>().begin());

        iterator begin()
        {
            return mR.begin();
        }

        iterator end()
        {
            return mR.end();
        }

        size_t size() const
        {
            return mR.size();
        }

        R mR;
    };

    template <typename R, typename F>
    struct TransformView {
        template <typename QualifiedR>
        struct iterator_impl {
            using It = decltype(std::declval<QualifiedR>().begin());
            using difference_type = typename It::difference_type;
            using value_type = std::decay_t<std::invoke_result_t<F, decltype(*std::declval<It>())>>;

            decltype(auto) operator*() const
            {
                return (*mF)(*mIt);
            }

            bool operator==(const iterator_impl &other) const
            {
                return mIt == other.mIt;
            }

            bool operator!=(const iterator_impl &other) const
            {
                return mIt != other.mIt;
            }

            iterator_impl &operator++()
            {
                ++mIt;
                return *this;
            }

            iterator_impl operator++(int)
            {
                iterator copy = *this;
                ++(*this);
                return copy;
            }

            It mIt;
            const std::remove_reference_t<F> *mF;
        };

        using iterator = iterator_impl<R>;
        using const_iterator = iterator_impl<const R>;

        using value_type = typename iterator::value_type;

        iterator begin()
        {
            return { mR.begin(), &mF };
        }

        iterator end()
        {
            return { mR.end(), &mF };
        }

        const_iterator begin() const
        {
            return { mR.begin(), &mF };
        }

        const_iterator end() const
        {
            return { mR.end(), &mF };
        }

        size_t size() const
        {
            return mR.size();
        }

        bool empty() const
        {
            return mR.empty();
        }

        R mR;
        F mF;
    };

    template <typename R, typename F>
    TransformView<R, F> transform(R &&r, F &&f)
    {
        return { std::forward<R>(r), std::forward<F>(f) };
    }

    MAKE_PIPABLE_FROM_RIGHT(transform)

    template <typename R>
    struct ReverseView {

        using iterator = std::reverse_iterator<decltype(std::declval<R>().begin())>;

        iterator begin()
        {
            return mR.rbegin();
        }

        iterator end()
        {
            return mR.rend();
        }

        R mR;
    };

    struct reverse_t {

        template <typename R>
        ReverseView<R> operator()(R &&r) const
        {
            return { std::forward<R>(r) };
        }

        template <typename R>
        friend ReverseView<R> operator|(R &&r, reverse_t)
        {
            return { std::forward<R>(r) };
        }
    };

    static constexpr reverse_t reverse;
}
}

    namespace views = ranges::views;

template <typename R>
inline constexpr bool ranges::enable_borrowed_range<views::ReverseView<R>> = ranges::borrowed_range<R>;

template <typename R, typename F>
inline constexpr bool ranges::enable_borrowed_range<views::TransformView<R, F>> = ranges::borrowed_range<R>;

#endif

}

