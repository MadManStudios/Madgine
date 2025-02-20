#pragma once

#include "../manuallifetime.h"

#include "emplace.h"

namespace Engine {

template <typename C, typename DataTraits = typename C::value_type>
struct FreeListContainer {

    using internal_container_type = typename container_traits<C>::template wrap_t<ManualLifetime>;
    using internal_traits = container_traits<internal_container_type>;
    using position_handle = typename internal_traits::position_handle;

    using value_type = typename C::value_type;
    using reference = typename C::reference;
    using const_reference = typename C::const_reference;
    using pointer = typename C::pointer;
    using const_pointer = typename C::const_pointer;

    template <typename It, typename _reference, typename _pointer>
    struct IteratorImpl {

        using iterator_category = typename It::iterator_category;

        using value_type = typename C::value_type;
        using difference_type = ptrdiff_t;
        using pointer = _pointer;
        using reference = _reference;

        IteratorImpl() = default;

        IteratorImpl(It it, const internal_container_type &cont)
            : mIt(std::move(it))
            , mContainer(&cont)
        {
            update();
        }

        template <typename It2, typename R2, typename P2>
        friend struct IteratorImpl;

        template <typename It2, typename R2, typename P2>
        IteratorImpl(const IteratorImpl<It2, R2, P2> &other)
            : mIt(other.mIt)
            , mContainer(other.mContainer)
        {
        }

        reference operator*() const
        {
            return *mIt;
        }

        pointer operator->() const
        {
            return &*mIt;
        }

        template <typename It2, typename R2, typename P2>
        difference_type operator-(const IteratorImpl<It2, R2, P2> &other) const
        {
            return mIt - other.mIt;
        }

        IteratorImpl operator+(difference_type diff) const
        {
            IteratorImpl result = *this;
            result += diff;
            return result;
        }

        IteratorImpl &operator+=(difference_type diff)
        {
            while (diff-- > 0)
                ++(*this);
            return *this;
        }

        bool operator!=(const IteratorImpl &other) const
        {
            assert(mContainer == other.mContainer);
            return mIt != other.mIt;
        }

        bool operator==(const IteratorImpl &other) const
        {
            assert(mContainer == other.mContainer);
            return mIt == other.mIt;
        }

        IteratorImpl &operator++()
        {
            ++mIt;
            update();
            return *this;
        }

        IteratorImpl operator++(int)
        {
            IteratorImpl other = *this;
            ++other;
            return other;
        }

        IteratorImpl &operator--()
        {
            --mIt;
            updateBack();
            return *this;
        }

        void update()
        {
            while (mIt != mContainer->end() && DataTraits::isFree(*mIt)) {
                ++mIt;
            }
        }

        void updateBack()
        {
            while (DataTraits::isFree(*mIt)) {
                --mIt;
            }
        }

        const It &it() const
        {
            return mIt;
        }

        const internal_container_type &container() const
        {
            return *mContainer;
        }

    private:
        It mIt;
        const internal_container_type *mContainer = nullptr;
    };

    using iterator = IteratorImpl<std::ranges::iterator_t<internal_container_type>, reference, pointer>;
    using const_iterator = IteratorImpl<std::ranges::const_iterator_t<internal_container_type>, const_reference, const_pointer>;

    iterator begin()
    {
        return { mContainer.begin(), mContainer };
    }

    const_iterator begin() const
    {
        return { mContainer.begin(), mContainer };
    }

    iterator end()
    {
        return { mContainer.end(), mContainer };
    }

    const_iterator end() const
    {
        return { mContainer.end(), mContainer };
    }

    using nodes_iterator = typename internal_traits::iterator;
    using nodes_const_iterator = typename internal_traits::const_iterator;

    nodes_iterator nodes_begin()
    {
        return { mContainer.begin(), mContainer };
    }

    nodes_const_iterator nodes_begin() const
    {
        return { mContainer.begin(), mContainer };
    }

    nodes_iterator nodes_end()
    {
        return { mContainer.end(), mContainer };
    }

    nodes_const_iterator nodes_end() const
    {
        return { mContainer.end(), mContainer };
    }

private:
    static position_handle &handle(typename internal_container_type::reference block)
    {
        assert(DataTraits::isFree(block));
        return *reinterpret_cast<position_handle *>(DataTraits::getLocation(block));
    }

    template <typename... Args>
    position_handle emplace(typename internal_container_type::reference block, Args &&...args)
    {
        assert(DataTraits::isFree(block));
        position_handle next = handle(block);
        handle(block).~position_handle();
        construct(block, std::forward<Args>(args)...);
        return next;
    }

    void destroy(typename internal_container_type::reference block, const position_handle &head)
    {
        assert(!DataTraits::isFree(block));
        destruct(block);
        new (&handle(block)) position_handle(head);
    }

public:
    ~FreeListContainer()
    {
        for (typename internal_container_type::reference block : mContainer) {
            if (!DataTraits::isFree(block)) {
                destruct(block);
            } else {
                handle(block).~position_handle();
            }
        }
    }

    template <typename... Ty>
    friend iterator tag_invoke(emplace_t, bool &success, FreeListContainer<C, DataTraits> &self, const const_iterator &where, Ty &&...args)
    {
        typename internal_traits::iterator it = internal_traits::toIterator(self.mContainer, self.mFreeListHead);

        assert(where.it() == self.mContainer.end() || where.it() == it);
        ++self.mSize;

        if (it == self.mContainer.end()) {
            it = Engine::emplace(success, self.mContainer, where.it());
            construct(*it, std::forward<Ty>(args)...);
            self.mFreeListHead = internal_traits::toPositionHandle(self.mContainer, self.mContainer.end());
        } else {
            self.mFreeListHead = self.emplace(*it, std::forward<Ty>(args)...);
        }
        success = true;
        return { it, self.mContainer };
    }

    void clear()
    {
        for (auto it = begin(); it != end(); ++it) {
            destroy(*it.it(), mFreeListHead);
            mFreeListHead = internal_traits::toPositionHandle(mContainer, it.it());
        }
        mSize = 0;
    }

    iterator erase(const iterator &where)
    {
        --mSize;
        destroy(*where.it(), mFreeListHead);
        mFreeListHead = internal_traits::toPositionHandle(mContainer, where.it());

        return { where.it(), mContainer };
    }

    template <typename... Args>
    value_type &emplace_back(Args &&...args)
    {
        return *emplace(end(), std::forward<Args>(args)...);
    }

    size_t size() const
    {
        return mSize;
    }

    size_t nodes_size() const
    {
        return mContainer.size();
    }

    reference at(size_t i)
    {
        return mContainer.at(i);
    }

    const_reference at(size_t i) const
    {
        return mContainer.at(i);
    }

    bool empty() const
    {
        return mSize == 0;
    }

private:
    friend struct container_traits<FreeListContainer<C, DataTraits>, void>;

    internal_container_type mContainer;
    position_handle mFreeListHead = internal_traits::toPositionHandle(mContainer, mContainer.end());
    size_t mSize = 0;
};

template <typename C, typename DataTraits>
struct underlying_container<FreeListContainer<C, DataTraits>> {
    typedef C type;
};

template <typename C, typename DataTraits>
struct container_traits<FreeListContainer<C, DataTraits>, void> : FreeListContainer<C, DataTraits>::internal_traits {

    typedef typename FreeListContainer<C, DataTraits>::iterator iterator;
    typedef typename FreeListContainer<C, DataTraits>::const_iterator const_iterator;

    static typename FreeListContainer<C, DataTraits>::internal_traits::position_handle toPositionHandle(FreeListContainer<C, DataTraits> &c, const iterator &it)
    {
        return FreeListContainer<C, DataTraits>::internal_traits::toPositionHandle(c.mContainer, it.it());
    }

    static typename FreeListContainer<C, DataTraits>::internal_traits::handle toHandle(FreeListContainer<C, DataTraits> &c, const iterator &it)
    {
        return FreeListContainer<C, DataTraits>::internal_traits::toHandle(c.mContainer, it.it());
    }

    static void revalidateHandleAfterInsert(typename FreeListContainer<C, DataTraits>::internal_traits::position_handle &handle, const FreeListContainer<C, DataTraits> &c, const const_iterator &it)
    {
        if (toIterator(c, handle) == it)
            FreeListContainer<C, DataTraits>::internal_traits::revalidateHandleAfterInsert(handle, c.mContainer, it.it());
    }

    static void revalidateHandleAfterRemove(typename FreeListContainer<C, DataTraits>::internal_traits::position_handle &handle, const FreeListContainer<C, DataTraits> &c, const const_iterator &it, bool wasIn, size_t count = 1)
    {
        /*size_t pivot = std::distance(c.begin(), it);
        if (pivot != handle)
            RefcountedContainer<C>::internal_traits::revalidateHandleAfterRemove(handle, c.mData, it.it(), wasIn, count);*/
    }

    static iterator toIterator(FreeListContainer<C, DataTraits> &c, const typename FreeListContainer<C, DataTraits>::internal_traits::position_handle &handle)
    {
        return { FreeListContainer<C, DataTraits>::internal_traits::toIterator(c.mContainer, handle), c.mContainer };
    }

    static const_iterator toIterator(const FreeListContainer<C, DataTraits> &c, const typename FreeListContainer<C, DataTraits>::internal_traits::const_position_handle &handle)
    {
        return { FreeListContainer<C, DataTraits>::internal_traits::toIterator(c.mContainer, handle), c.mContainer };
    }

    static typename FreeListContainer<C, DataTraits>::internal_traits::position_handle next(FreeListContainer<C, DataTraits> &c, const typename FreeListContainer<C, DataTraits>::internal_traits::position_handle &handle)
    {
        return toPositionHandle(c, ++toIterator(c, handle));
    }

    static typename FreeListContainer<C, DataTraits>::internal_traits::position_handle prev(FreeListContainer<C, DataTraits> &c, const typename FreeListContainer<C, DataTraits>::internal_traits::position_handle &handle)
    {
        return toPositionHandle(c, --toIterator(c, handle));
    }
};

}