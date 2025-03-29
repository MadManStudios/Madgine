#pragma once

#include "../indextype.h"
#include "underlying_container.h"

DERIVE_FUNCTION(isReference)

namespace Engine {

template <typename C, typename = void>
struct container_traits;

template <typename C>
requires requires { typename underlying_container<C>::type; }
struct container_traits<C> : container_traits<typename underlying_container<C>::type>
{
    typedef C container;
};

template <typename T>
struct container_traits<std::list<T>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;
    static constexpr const bool is_fixed_size = false;

    typedef typename std::list<T>::iterator iterator;
    typedef typename std::list<T>::const_iterator const_iterator;
    typedef T *handle;
    typedef const T *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;

    //static_assert(sizeof(position_handle) <= sizeof(void *));

    static position_handle toPositionHandle(std::list<T> &c, const iterator &it)
    {
        return it;
    }

    static handle toHandle(std::list<T> &c, const iterator &it)
    {
        if (it == c.end())
            return nullptr;
        return &*it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const std::list<T> &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const std::list<T> &c, const iterator &it, bool wasIn, size_t count = 1)
    {
        if (wasIn)
            handle = it;
    }

    static iterator toIterator(std::list<T> &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const std::list<T> &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(std::list<T> &c, const position_handle &handle)
    {
        return std::next(handle);
    }

    static position_handle prev(std::list<T> &c, const position_handle &handle)
    {
        return std::prev(handle);
    }
};

template <typename T>
struct container_traits<std::vector<T>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = true;
    static constexpr const bool remove_invalidates_handles = true;
    static constexpr const bool is_fixed_size = false;

    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    typedef IndexType<size_t> handle;
    typedef IndexType<size_t> const_handle;
    typedef IndexType<size_t> position_handle;
    typedef IndexType<size_t> const_position_handle;

    static_assert(sizeof(position_handle) <= sizeof(void *));

    static position_handle toPositionHandle(std::vector<T> &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static handle toHandle(std::vector<T> &c, const position_handle &handle)
    {
        return handle;
    }

    static handle toHandle(std::vector<T> &c, const iterator &it)
    {
        if (it == c.end())
            return {};
        return std::distance(c.begin(), it);
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const std::vector<T> &c, const const_iterator &it)
    {
        size_t item = std::distance(c.begin(), it);
        if (item <= handle)
            ++handle;
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const std::vector<T> &c, const const_iterator &it, bool wasIn, size_t count = 1)
    {
        size_t pivot = std::distance(c.begin(), it);
        if (wasIn) {
            handle = pivot;
        } else {
            assert(handle < pivot || handle >= pivot + count);
            if (handle > pivot)
                handle -= count;
        }
    }

    static iterator toIterator(std::vector<T> &c, const position_handle &handle)
    {
        return c.begin() + handle;
    }

    static const_iterator toIterator(const std::vector<T> &c, const const_position_handle &handle)
    {
        return c.begin() + handle;
    }

    static position_handle next(std::vector<T> &c, const position_handle &handle)
    {
        return handle + 1;
    }

    static position_handle prev(std::vector<T> &c, const position_handle &handle)
    {
        return handle - 1;
    }
};

template <typename T, typename Cmp>
struct container_traits<std::set<T, Cmp>> {
    static constexpr const bool sorted = true;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;
    static constexpr const bool is_fixed_size = false;

    typedef typename std::set<T, Cmp>::iterator iterator;
    typedef typename std::set<T, Cmp>::const_iterator const_iterator;
    typedef const T *handle;
    typedef const T *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;

    //static_assert(sizeof(position_handle) <= sizeof(void *));

    static position_handle toPositionHandle(std::set<T, Cmp> &c, const iterator &it)
    {
        return it;
    }

    static handle toHandle(std::set<T, Cmp> &c, const iterator &it)
    {
        if (it == c.end())
            return nullptr;
        return &*it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const std::set<T, Cmp> &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const std::set<T, Cmp> &c, const iterator &it, bool wasIn, size_t count = 1)
    {
        if (wasIn)
            handle = it;
    }

    static iterator toIterator(std::set<T, Cmp> &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const std::set<T, Cmp> &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(std::set<T, Cmp> &c, const position_handle &handle)
    {
        return std::next(handle);
    }

    static position_handle prev(std::set<T, Cmp> &c, const position_handle &handle)
    {
        return std::prev(handle);
    }
};

template <typename K, typename T, typename Cmp>
struct container_traits<std::map<K, T, Cmp>> {
    static constexpr const bool sorted = true;
    static constexpr const bool has_dependent_handle = false;
    static constexpr const bool remove_invalidates_handles = false;
    static constexpr const bool is_fixed_size = false;

    typedef typename std::map<K, T, Cmp>::iterator iterator;
    typedef typename std::map<K, T, Cmp>::const_iterator const_iterator;
    typedef std::pair<const K, T> *handle;
    typedef const std::pair<const K, T> *const_handle;
    typedef iterator position_handle;
    typedef const_iterator const_position_handle;

    //static_assert(sizeof(position_handle) <= sizeof(void *));

    static position_handle toPositionHandle(std::map<K, T, Cmp> &c, const iterator &it)
    {
        return it;
    }

    static handle toHandle(std::map<K, T, Cmp> &c, const iterator &it)
    {
        if (it == c.end())
            return nullptr;
        return &*it;
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const std::map<K, T, Cmp> &c, const const_iterator &it)
    {
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const std::map<K, T, Cmp> &c, const iterator &it, bool wasIn, size_t count = 1)
    {
        if (wasIn)
            handle = it;
    }

    static iterator toIterator(std::map<K, T, Cmp> &c, const position_handle &handle)
    {
        return handle;
    }

    static const_iterator toIterator(const std::map<K, T, Cmp> &c, const const_position_handle &handle)
    {
        return handle;
    }

    static position_handle next(std::map<K, T, Cmp> &c, const position_handle &handle)
    {
        return std::next(handle);
    }

    static position_handle prev(std::map<K, T, Cmp> &c, const position_handle &handle)
    {
        return std::prev(handle);
    }
};

template <typename T, size_t Size>
struct container_traits<std::array<T, Size>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = true;
    static constexpr const bool remove_invalidates_handles = false;
    static constexpr const bool is_fixed_size = true;

    typedef typename std::array<T, Size>::iterator iterator;
    typedef typename std::array<T, Size>::const_iterator const_iterator;

    typedef IndexType<uint32_t> handle;
    typedef IndexType<uint32_t> const_handle;
    typedef IndexType<uint32_t> position_handle;
    typedef IndexType<uint32_t> const_position_handle;
    
    static_assert(sizeof(position_handle) <= sizeof(void *));

    /* template <typename C>
    struct api : C {

        using C::C;

        using C::operator=;

        value_type &at(size_t i)
        {
            return C::at(i);
        }

        const value_type &at(size_t i) const
        {
            return C::at(i);
        }

        value_type &operator[](size_t i)
        {
            return C::operator[](i);
        }

        const value_type &operator[](size_t i) const
        {
            return C::operator[](i);
        }
    };*/

    static position_handle toPositionHandle(std::array<T, Size> &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static handle toHandle(std::array<T, Size> &c, const position_handle &handle)
    {
        return handle;
    }

    static handle toHandle(std::array<T, Size> &c, const iterator &it)
    {
        if (it == c.end())
            return {};
        return std::distance(c.begin(), it);
    }

    static iterator toIterator(std::array<T, Size> &c, const position_handle &handle)
    {
        return c.begin() + handle;
    }

    static const_iterator toIterator(const std::array<T, Size> &c, const const_position_handle &handle)
    {
        return c.begin() + handle;
    }

    static position_handle next(std::array<T, Size> &c, const position_handle &handle)
    {
        return handle + 1;
    }

    static position_handle prev(std::array<T, Size> &c, const position_handle &handle)
    {
        return handle - 1;
    }
};

template <typename T>
struct container_traits<std::deque<T>> {
    static constexpr const bool sorted = false;
    static constexpr const bool has_dependent_handle = true;
    static constexpr const bool remove_invalidates_handles = true;
    static constexpr const bool is_fixed_size = false;

    typedef typename std::deque<T>::iterator iterator;
    typedef typename std::deque<T>::const_iterator const_iterator;

    typedef IndexType<size_t> handle;
    typedef IndexType<size_t> const_handle;
    typedef IndexType<size_t> position_handle;
    typedef IndexType<size_t> const_position_handle;

    static_assert(sizeof(position_handle) <= sizeof(void *));

    template <typename U>
    using rebind_t = std::deque<U>;

    template <template <typename> typename W>
    using wrap_t = std::deque<W<T>>;

    static position_handle toPositionHandle(std::deque<T> &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static handle toHandle(std::deque<T> &c, const position_handle &handle)
    {
        return handle;
    }

    static handle toHandle(std::deque<T> &c, const iterator &it)
    {
        if (it == c.end())
            return {};
        return std::distance(c.begin(), it);
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const std::deque<T> &c, const const_iterator &it)
    {
        size_t item = std::distance(c.begin(), it);
        if (item <= handle)
            ++handle;
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const std::deque<T> &c, const const_iterator &it, bool wasIn, size_t count = 1)
    {
        size_t pivot = std::distance(c.begin(), it);
        if (wasIn) {
            handle = pivot;
        } else {
            assert(handle < pivot || handle >= pivot + count);
            if (handle > pivot)
                handle -= count;
        }
    }

    static iterator toIterator(std::deque<T> &c, const position_handle &handle)
    {
        return c.begin() + handle;
    }

    static const_iterator toIterator(const std::deque<T> &c, const const_position_handle &handle)
    {
        return c.begin() + handle;
    }

    static position_handle next(std::deque<T> &c, const position_handle &handle)
    {
        return handle + 1;
    }

    static position_handle prev(std::deque<T> &c, const position_handle &handle)
    {
        return handle - 1;
    }
};

}
