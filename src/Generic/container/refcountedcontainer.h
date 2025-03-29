#pragma once

#include "freelistcontainer.h"

namespace Engine {

template <typename C>
struct RefcountedContainer {

    using value_type = typename C::value_type;
    using reference = typename C::reference;
    using const_reference = typename C::const_reference;

    struct ControlBlock {

        template <typename... Args>
        ControlBlock(Args &&...args)
        {
            new (&mData) value_type(std::forward<Args>(args)...);
        }

        ControlBlock(ControlBlock &&other)
        {
            throw 0;
        }

        ControlBlock &operator=(ControlBlock &&other)
        {
            throw 0;
        }

        ~ControlBlock()
        {
            if (!mDeadFlag) {
                destroy();
            }
            mFree = true;
        }

        void destroy()
        {
            assert(!mDeadFlag);
            mData.~value_type();
            mDeadFlag = true;
        }

        value_type *get()
        {
            return &mData;
        }

        const value_type *get() const
        {
            return &mData;
        }

        operator value_type &()
        {
            return *get();
        }

        operator const value_type &() const
        {
            return *get();
        }

        static ControlBlock *fromPtr(value_type *ptr)
        {
            if (!ptr)
                return nullptr;
            return reinterpret_cast<ControlBlock *>(ptr);
        }

        void incRef();
        void decRef();

        bool dead() const
        {
            return mDeadFlag;
        }

        static bool isFree(const ManualLifetime<ControlBlock> &block)
        {
            return block.unsafeAccess().mFree;
        }

        static uintptr_t *getLocation(ManualLifetime<ControlBlock> &block)
        {
            return &block.unsafeAccess().mBuffer;
        }

    private:
        union {
            value_type mData;
            struct {
                RefcountedContainer *mContainer;
                uintptr_t mBuffer;
            };
        };
        bool mDeadFlag : 1 = false;
        bool mFree : 1 = false;
        uint32_t mRefCount : 30 = 0;

        friend struct RefcountedContainer<C>;
    };

    static_assert(sizeof(uintptr_t) + sizeof(RefcountedContainer *) <= sizeof(value_type));

    using internal_container_type = FreeListContainer<typename container_traits<C>::template rebind_t<ControlBlock>>;
    using internal_traits = container_traits<internal_container_type>;
    using position_handle = typename internal_traits::position_handle;

public:
    template <typename It, typename Val, bool skipping>
    struct IteratorImpl {

        using iterator_category = typename It::iterator_category;

        using value_type = value_type;
        using difference_type = ptrdiff_t;
        using pointer = Val *;
        using reference = Val &;

        IteratorImpl() = default;

        IteratorImpl(It it)
            : mIt(std::move(it))
        {
            if constexpr (skipping)
                update();
        }

        template <typename It2, typename Val2, bool skipping2>
        friend struct IteratorImpl;

        template <typename It2, typename Val2>
        IteratorImpl(const IteratorImpl<It2, Val2, skipping> &other)
            : mIt(other.mIt)
        {
        }

        reference operator*() const
        {
            return *mIt;
        }

        pointer operator->() const
        {
            return &static_cast<reference>(*mIt);
        }

        decltype(auto) get_block() const
        {
            return *mIt;
        }

        difference_type operator-(const IteratorImpl &other) const
        {
            return mIt - other.mIt;
        }

        IteratorImpl operator+(difference_type diff) const
        {
            return { mIt + diff };
        }

        IteratorImpl &operator+=(difference_type diff)
        {
            mIt += diff;
            if constexpr (skipping)
                update();
            return *this;
        }

        bool operator!=(const IteratorImpl &other) const
        {
            return mIt != other.mIt;
        }

        bool operator==(const IteratorImpl &other) const
        {
            return mIt == other.mIt;
        }

        IteratorImpl &operator++()
        {
            ++mIt;
            if constexpr (skipping)
                update();
            return *this;
        }

        IteratorImpl operator++(int)
        {
            IteratorImpl copy = *this;
            ++*this;
            return copy;
        }

        IteratorImpl &operator--()
        {
            --mIt;
            if constexpr (skipping)
                updateBack();
            return *this;
        }

        void update()
        {
            while (mIt.it() != mIt.container().end() && mIt->dead()) {
                ++mIt;
            }
        }

        void updateBack()
        {
            while (mIt->dead()) {
                --mIt;
            }
        }

        const It &it() const
        {
            return mIt;
        }

    private:
        It mIt;
    };

    using iterator = IteratorImpl<typename internal_traits::iterator, value_type, true>;
    using const_iterator = IteratorImpl<typename internal_traits::const_iterator, const value_type, true>;    

    iterator begin()
    {
        return { mData.begin() };
    }

    const_iterator begin() const
    {
        return { mData.begin() };
    }

    iterator end()
    {
        return { mData.end() };
    }

    const_iterator end() const
    {
        return { mData.end() };
    }

    using refcounted_iterator = IteratorImpl<typename internal_container_type::iterator, ControlBlock, false>;    
    using refcounted_const_iterator = IteratorImpl<typename internal_container_type::const_iterator, const ControlBlock, false>;

    refcounted_iterator refcounted_begin()
    {
        return { mData.begin() };
    }

    refcounted_const_iterator refcounted_begin() const
    {
        return { mData.begin() };
    }

    refcounted_iterator refcounted_end()
    {
        return { mData.end() };
    }

    refcounted_const_iterator refcounted_end() const
    {
        return { mData.end() };
    }

    using blocks_iterator = IteratorImpl<typename internal_container_type::nodes_iterator, ControlBlock, false>;    
    using blocks_const_iterator = IteratorImpl<typename internal_container_type::nodes_const_iterator, const ControlBlock, false>;

    blocks_iterator blocks_begin()
    {
        return { mData.nodes_begin() };
    }

    blocks_const_iterator blocks_begin() const
    {
        return { mData.nodes_begin() };
    }

    blocks_iterator blocks_end()
    {
        return { mData.nodes_end() };
    }

    blocks_const_iterator blocks_end() const
    {
        return { mData.nodes_end() };
    }

    struct RefcountedView {
        RefcountedContainer<C> &mData;
        refcounted_iterator begin() const
        {
            return mData.refcounted_begin();
        }
        refcounted_iterator end() const
        {
            return mData.refcounted_end();
        }
        size_t size() const
        {
            return mData.size();
        }
        constexpr bool isReference() const
        {
            return true;
        }
    };

    RefcountedView refcounted()
    {
        return { *this };
    }

    struct BlocksView {
        RefcountedContainer<C> &mData;
        blocks_iterator begin() const
        {
            return mData.blocks_begin();
        }
        blocks_iterator end() const
        {
            return mData.blocks_end();
        }
        size_t size() const
        {
            return mData.size();
        }
        constexpr bool isReference() const
        {
            return true;
        }
    };

    BlocksView blocks()
    {
        return { *this };
    }

    template <typename... Ty>
    friend iterator tag_invoke(emplace_t, bool &success, RefcountedContainer<C> &self, const const_iterator &where, Ty &&...args)
    {
        return Engine::emplace(success, self.mData, where.it(), std::forward<Ty>(args)...);
    }

    void clear()
    {
        for (auto it = begin(); it != end();) {
            it = erase(it);
        }
    }

    iterator erase(const iterator &where)
    {
        if (where.get_block().mRefCount == 0) {
            return mData.erase(where.it());
        } else {
            where.get_block().destroy();
            where.get_block().mContainer = this;
            new (&where.get_block().mBuffer) position_handle(internal_traits::toPositionHandle(mData, where.it()));
            return std::next(where);
        }
    }

    refcounted_iterator erase_refcounted(const refcounted_iterator &where)
    {
        if (where->mRefCount == 0) {
            return mData.erase(where.it());
        } else {
            where->destroy();
            where->mContainer = this;
            new (&where->mBuffer) position_handle(internal_traits::toPositionHandle(mData, where.it()));
            return std::next(where);
        }
    }

    reference at(size_t i)
    {
        return mData.at(i);
    }

    const_reference at(size_t i) const
    {
        return mData.at(i);
    }

    size_t size() const
    {
        return mData.size();
    }

    bool empty() const
    {
        return mData.empty();
    }

private:
    friend struct container_traits<RefcountedContainer<C>, void>;

    internal_container_type mData;
};

template <typename C>
void RefcountedContainer<C>::ControlBlock::incRef()
{
    ++mRefCount;
}

template <typename C>
void RefcountedContainer<C>::ControlBlock::decRef()
{
    --mRefCount;
    if (mRefCount == 0 && mDeadFlag)
        mContainer->erase_refcounted(internal_traits::toIterator(mContainer->mData, *reinterpret_cast<position_handle *>(&mBuffer)));
}

template <typename C>
struct underlying_container<RefcountedContainer<C>> {
    typedef C type;
};

template <typename C>
struct container_traits<RefcountedContainer<C>> : RefcountedContainer<C>::internal_traits {
    
    typedef typename RefcountedContainer<C>::iterator iterator;
    typedef typename RefcountedContainer<C>::const_iterator const_iterator;

    static typename RefcountedContainer<C>::internal_traits::position_handle toPositionHandle(RefcountedContainer<C> &c, const iterator &it)
    {
        return RefcountedContainer<C>::internal_traits::toPositionHandle(c.mData, it.it());
    }

    static typename RefcountedContainer<C>::internal_traits::handle toHandle(RefcountedContainer<C> &c, const iterator &it)
    {
        return RefcountedContainer<C>::internal_traits::toHandle(c.mData, it.it());
    }

    static void revalidateHandleAfterInsert(typename RefcountedContainer<C>::internal_traits::position_handle &handle, const RefcountedContainer<C> &c, const const_iterator &it)
    {
        if (toIterator(c, handle) == it)
            RefcountedContainer<C>::internal_traits::revalidateHandleAfterInsert(handle, c.mData, it.it());
    }

    static void revalidateHandleAfterRemove(typename RefcountedContainer<C>::internal_traits::position_handle &handle, const RefcountedContainer<C> &c, const const_iterator &it, bool wasIn, size_t count = 1)
    {
        /*size_t pivot = std::distance(c.begin(), it);
        if (pivot != handle)
            RefcountedContainer<C>::internal_traits::revalidateHandleAfterRemove(handle, c.mData, it.it(), wasIn, count);*/
    }

    static iterator toIterator(RefcountedContainer<C> &c, const typename RefcountedContainer<C>::internal_traits::position_handle &handle)
    {
        return { RefcountedContainer<C>::internal_traits::toIterator(c.mData, handle) };
    }

    static const_iterator toIterator(const RefcountedContainer<C> &c, const typename RefcountedContainer<C>::internal_traits::const_position_handle &handle)
    {
        return { RefcountedContainer<C>::internal_traits::toIterator(c.mData, handle) };
    }

    static typename RefcountedContainer<C>::internal_traits::position_handle next(RefcountedContainer<C> &c, const typename RefcountedContainer<C>::internal_traits::position_handle &handle)
    {
        return toPositionHandle(c, ++toIterator(c, handle));
    }

    static typename RefcountedContainer<C>::internal_traits::position_handle prev(RefcountedContainer<C> &c, const typename RefcountedContainer<C>::internal_traits::position_handle &handle)
    {
        return toPositionHandle(c, --toIterator(c, handle));
    }
};

}