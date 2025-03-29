#pragma once

#include "../proxy.h"

#include "../defaultassign.h"

#include "../heapobject.h"

#include "emplace.h"

namespace Engine {

struct VirtualSentinel {
};

namespace __generic_impl__ {

    template <typename RefT>
    struct VirtualRangeBase;

    template <typename RefT>
    struct VirtualIteratorBase {
        virtual ~VirtualIteratorBase() = default;
        virtual void increment() = 0;
        virtual void getValue(RefT &ref) const = 0;
        virtual std::unique_ptr<VirtualIteratorBase<RefT>> clone() const = 0;
        virtual bool ended() const = 0;

        const std::shared_ptr<VirtualRangeBase<RefT>> &range() const
        {
            return mRange;
        }

    protected:
        VirtualIteratorBase(std::shared_ptr<VirtualRangeBase<RefT>> range)
            : mRange(std::move(range))
        {
        }

        std::shared_ptr<VirtualRangeBase<RefT>> mRange;
    };

    template <typename RefT, typename It, typename EndIt, typename Assign>
    struct VirtualIteratorImpl : VirtualIteratorBase<RefT> {

        template <typename It2, typename EndIt2>
        VirtualIteratorImpl(It2 &&it, EndIt2 &&end, std::shared_ptr<VirtualRangeBase<RefT>> range)
            : VirtualIteratorBase<RefT>(std::move(range))
            , mIt(std::forward<It2>(it))
            , mEnd(std::forward<EndIt2>(end))
        {
        }

        virtual void increment() override
        {
            ++mIt;
        }

        virtual void getValue(RefT &ref) const override
        {
            Assign {}(ref, *mIt);
        }

        virtual std::unique_ptr<VirtualIteratorBase<RefT>> clone() const override
        {
            return std::make_unique<VirtualIteratorImpl<RefT, It, EndIt, Assign>>(mIt, mEnd, this->mRange);
        }

        virtual bool ended() const override
        {
            return mIt == mEnd;
        }

        It get() const
        {
            return mIt;
        }

    private:
        It mIt;
        EndIt mEnd;
    };

}

template <typename RefT>
struct VirtualIterator {

    using iterator_category = std::input_iterator_tag;
    using value_type = RefT;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = RefT;

    VirtualIterator() = default;

    VirtualIterator(std::unique_ptr<__generic_impl__::VirtualIteratorBase<RefT>> impl)
        : mImpl(std::move(impl))
    {
    }

    VirtualIterator(const VirtualIterator<RefT> &other)
        : mImpl(other.mImpl->clone())
    {
    }

    VirtualIterator(VirtualIterator<RefT> &&other)
        : mImpl(std::move(other.mImpl))
    {
    }

    VirtualIterator<RefT> &operator=(const VirtualIterator<RefT> &other)
    {
        mImpl = other.mImpl->clone();
        return *this;
    }

    VirtualIterator<RefT> &operator=(VirtualIterator<RefT> &&other)
    {
        mImpl = std::move(other.mImpl);
        return *this;
    }

    VirtualIterator<RefT> &operator++()
    {
        mImpl->increment();
        return *this;
    }

    VirtualIterator<RefT> operator++(int)
    {
        VirtualIterator<RefT> temp = *this;
        ++*this;
        return temp;
    }

    RefT operator*() const
    {
        RefT result;
        mImpl->getValue(result);
        return result;
    }

    Proxy<RefT, false> operator->() const
    {
        RefT result;
        mImpl->getValue(result);
        return result;
    }

    bool operator==(const VirtualSentinel &other) const
    {
        return ended();
    }

    friend bool operator==(const VirtualSentinel &other, const VirtualIterator &it)
    {
        return it.ended();
    }

    bool operator!=(const VirtualSentinel &other) const
    {
        return !ended();
    }

    friend bool operator!=(const VirtualSentinel &other, const VirtualIterator &it)
    {
        return !it.ended();
    }

    VirtualRange<RefT> range() const
    {
        return mImpl->range();
    }

    bool ended() const
    {
        return mImpl->ended();
    }

    __generic_impl__::VirtualIteratorBase<RefT> *get() const
    {
        return mImpl.get();
    }

private:
    std::unique_ptr<__generic_impl__::VirtualIteratorBase<RefT>> mImpl;
};

namespace __generic_impl__ {

    template <typename RefT>
    struct VirtualRangeBase {
        virtual ~VirtualRangeBase() = default;
        virtual VirtualIterator<RefT> begin(std::shared_ptr<VirtualRangeBase<RefT>> self) = 0;
        virtual VirtualIterator<RefT> insert(const VirtualIterator<RefT> &where, std::shared_ptr<VirtualRangeBase<RefT>> self) = 0;
        virtual VirtualIterator<RefT> insert(const VirtualSentinel &where, std::shared_ptr<VirtualRangeBase<RefT>> self) = 0;
        virtual bool isReference() const = 0;
        virtual bool canInsert() const = 0;
    };

    template <typename RefT, typename C>
    struct VirtualRangeSecondBase : VirtualRangeBase<RefT> {
        virtual C &get() = 0;
    };

    template <typename RefT, typename C, typename Assign>
    struct VirtualRangeImpl : VirtualRangeSecondBase<RefT, std::remove_reference_t<C>> {

        using It = decltype(std::declval<C>().begin());
        using Sentinel = decltype(std::declval<C>().end());
        using IteratorImpl = __generic_impl__::VirtualIteratorImpl<RefT, It, Sentinel, Assign>;

        static constexpr bool sCanInsert = requires(C &c) {
            emplace(c, c.end());
        };

        VirtualRangeImpl(C &&c)
            : mContainer(std::forward<C>(c))
        {
        }

        virtual VirtualIterator<RefT> begin(std::shared_ptr<VirtualRangeBase<RefT>> self) override
        {
            return { std::make_unique<IteratorImpl>(mContainer.begin(), mContainer.end(), std::move(self)) };
        }

        virtual VirtualIterator<RefT> insert(const VirtualIterator<RefT> &where, std::shared_ptr<VirtualRangeBase<RefT>> self) override
        {
            if constexpr (sCanInsert) {
                VirtualIteratorBase<RefT> *innerIt = where.get();
                assert(dynamic_cast<IteratorImpl *>(innerIt));
                auto it = emplace(mContainer, static_cast<IteratorImpl *>(innerIt)->get());
                return { std::make_unique<IteratorImpl>(it, mContainer.end(), self) };
            } else {
                throw 0;
            }
        }

        virtual VirtualIterator<RefT> insert(const VirtualSentinel &where, std::shared_ptr<VirtualRangeBase<RefT>> self) override
        {
            if constexpr (sCanInsert) {
                auto it = emplace(mContainer, mContainer.end());
                return { std::make_unique<IteratorImpl>(it, mContainer.end(), self) };
            } else {
                throw 0;
            }
        }

        virtual C &get() override
        {
            return mContainer;
        }

        virtual bool isReference() const override
        {
            if constexpr (std::is_reference_v<C> || InstanceOf<std::ranges::range_value_t<C>, std::reference_wrapper>)
                return true;
            else if constexpr (has_function_isReference_v<C>)
                return mContainer.isReference();
            else
                return false;
        }

        virtual bool canInsert() const override
        {
            return sCanInsert;
        }

    private:
        C mContainer;
    };

}

template <typename RefT, typename AssignDefault>
struct VirtualRange {

    template <typename C, typename Assign = AssignDefault>
        requires(!std::same_as<std::decay_t<C>, VirtualRange<RefT, AssignDefault>>)
    explicit VirtualRange(C &&c, type_holder_t<Assign> = {})
        : mRange(std::make_shared<__generic_impl__::VirtualRangeImpl<RefT, C, Assign>>(std::forward<C>(c)))
    {
    }

    VirtualRange(std::shared_ptr<__generic_impl__::VirtualRangeBase<RefT>> range)
        : mRange(std::move(range))
    {
    }

    VirtualRange(const VirtualRange &) = default;
    VirtualRange(VirtualRange &&) = default;

    template <typename Assign, typename C>
    void assign(C &&c)
    {
        mRange = std::make_shared<__generic_impl__::VirtualRangeImpl<RefT, C, Assign>>(std::forward<C>(c));
    }

    template <typename C>
        requires(!std::same_as<std::decay_t<C>, VirtualRange<RefT, AssignDefault>>)
    VirtualRange &
    operator=(C &&c)
    {
        assign<AssignDefault>(std::forward<C>(c));
        return *this;
    }

    VirtualRange &operator=(const VirtualRange &) = default;
    VirtualRange &operator=(VirtualRange &&) = default;

    VirtualIterator<RefT> begin() const
    {
        return mRange->begin(mRange);
    }

    VirtualSentinel end() const
    {
        return {};
    }

    VirtualIterator<RefT> insert(const VirtualIterator<RefT> &where)
    {
        return mRange->insert(where, mRange);
    }

    VirtualIterator<RefT> insert(const VirtualSentinel &where)
    {
        return mRange->insert(where, mRange);
    }

    template <typename C>
    HeapObject<C, std::shared_ptr<C>> safe_cast()
    {
        __generic_impl__::VirtualRangeSecondBase<RefT, C> *typed = dynamic_cast<__generic_impl__::VirtualRangeSecondBase<RefT, C> *>(mRange.get());
        if (typed) {
            if (isReference()) {
                return std::shared_ptr<C> { &typed->get(), [](C *) {} };
            } else {
                return std::shared_ptr<C> { mRange, &typed->get() };
            }
        } else {
            std::shared_ptr<C> result = std::make_shared<C>();
            throw 0;
            return result;
        }
    }

    template <typename C>
    HeapObject<const C, std::shared_ptr<const C>> safe_cast() const
    {
        __generic_impl__::VirtualRangeSecondBase<RefT, C> *typed = dynamic_cast<__generic_impl__::VirtualRangeSecondBase<RefT, C> *>(mRange.get());
        if (typed) {
            if (isReference()) {
                return std::shared_ptr<const C> { &typed->get(), [](C *) {} };
            } else {
                return std::shared_ptr<const C> { mRange, &typed->get() };
            }
        } else {
            std::shared_ptr<C> result = std::make_shared<C>();
            throw 0;
            return std::shared_ptr<const C> { result };
        }
    }

    template <typename C>
    C &unsafe_cast()
    {
        return dynamic_cast<__generic_impl__::VirtualRangeSecondBase<RefT, C> *>(mRange.get())->get();
    }

    template <typename C>
    const C &unsafe_cast() const
    {
        return dynamic_cast<__generic_impl__::VirtualRangeSecondBase<RefT, C> *>(mRange.get())->get();
    }

    bool isReference() const
    {
        return mRange->isReference();
    }

    bool canInsert() const
    {
        return mRange->canInsert();
    }

    bool operator==(const VirtualRange &other) const
    {
        throw "TODO";
        return false;
        // return std::equal(begin(), end(), other.begin(), other.end());
    }

private:
    std::shared_ptr<__generic_impl__::VirtualRangeBase<RefT>> mRange;
};
}