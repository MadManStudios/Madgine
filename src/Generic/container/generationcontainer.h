#pragma once 

#if ENABLE_MEMTRACKING
#    include "Interfaces/debug/stacktrace.h"
#endif

#include "containerevent.h"

namespace Engine {

struct GenerationContainerIndex {

    GenerationContainerIndex() = default;
    GenerationContainerIndex(const GenerationContainerIndex &) = delete;
    GenerationContainerIndex(GenerationContainerIndex &&other)
        : mIndex(std::move(other.mIndex))
        , mGeneration(std::move(other.mGeneration))
#if ENABLE_MEMTRACKING
        , mDebugMarker(std::exchange(other.mDebugMarker, 0))
#endif
    {
    }

    ~GenerationContainerIndex()
    {
        assert(!*this);
#if ENABLE_MEMTRACKING
        assert(mDebugMarker == 0);
#endif
    }

    explicit operator bool() const
    {
        if (mGeneration) {
            return true;
        } else {
            assert(!mIndex);
            return false;
        }
    }

    bool operator==(const GenerationContainerIndex &other) const
    {
        if (!mGeneration || !other.mGeneration)
            return mGeneration == other.mGeneration;
        assert(mGeneration == other.mGeneration);
        return mIndex == other.mIndex;
    }

    bool operator!=(const GenerationContainerIndex &other) const
    {
        if (!mGeneration || !other.mGeneration)
            return mGeneration != other.mGeneration;
        assert(mGeneration == other.mGeneration);
        return mIndex != other.mIndex;
    }

    void operator=(GenerationContainerIndex &&other)
    {
        std::swap(mIndex, other.mIndex);
        std::swap(mGeneration, other.mGeneration);
#if ENABLE_MEMTRACKING
        std::swap(mDebugMarker, other.mDebugMarker);
#endif
    }

private:
    friend struct GenerationContainerBase;

    GenerationContainerIndex(uint32_t index, uint32_t generation)
        : mIndex(index)
        , mGeneration(generation)
    {
    }

    void reset()
    {
        mIndex.reset();
        mGeneration.reset();
    }

    IndexType<uint32_t> mIndex;
    IndexType<uint16_t> mGeneration;

#if ENABLE_MEMTRACKING
    mutable uint32_t mDebugMarker = 0;
#endif
};

struct GenerationContainerBase {
    GenerationContainerBase(uint16_t bufferSize = 64)
        : mHistory(bufferSize)
    {
    }

    GenerationContainerBase(const GenerationContainerBase &) = delete;
    GenerationContainerBase(GenerationContainerBase &&) = delete;

    ~GenerationContainerBase()
    {
        for (Entry &entry : mHistory)
            assert(entry.mRefCount == 0);
    }

    GenerationContainerIndex generate(uint32_t index) const
    {
        uint16_t gen = mGeneration.load();
        incRef(gen);
        GenerationContainerIndex result { index, gen };
#if ENABLE_MEMTRACKING
        std::lock_guard lock { mDebugMutex };
        result.mDebugMarker = ++mDebugCounter;
        auto pib = mDebugTraces.try_emplace(result.mDebugMarker, std::make_pair(gen, Debug::StackTrace<8>::getCurrent(1)));
        assert(pib.second);
#endif

        return result;
    }

    void reset(GenerationContainerIndex &index) const
    {
        if (index) {
            decRef(index.mGeneration);
#if ENABLE_MEMTRACKING
            {
                std::lock_guard lock { mDebugMutex };
                auto count = mDebugTraces.erase(index.mDebugMarker);
                assert(count == 1);
                index.mDebugMarker = 0;
            }
#endif
            index.reset();
        }
    }

    GenerationContainerIndex copy(GenerationContainerIndex &other) const
    {
        update(other);
        if (!other)
            return {};
        return generate(other.mIndex);
    }

    void increment(GenerationContainerIndex &index, size_t size, ptrdiff_t diff = 1) const
    {
        update(index);
        assert(index || diff == 0);
        index.mIndex += diff;
        assert(index.mIndex <= size || !index.mIndex);
        if (index.mIndex == size || !index.mIndex)
            reset(index);
    }

    uint32_t update(GenerationContainerIndex &index) const
    {
        uint32_t oldIndex = index.mIndex;
        if (index && index.mGeneration != mGeneration) {

            uint32_t oldGen = index.mGeneration;

            //Apply History
            uint32_t wrap = mHistory.size();
            while (index.mGeneration != mGeneration) {

                const Entry &entry = mHistory[index.mGeneration];

                if (entry.mOp == RESET) {
                    index.reset();
#if ENABLE_MEMTRACKING
                    std::lock_guard lock { mDebugMutex };
                    auto count = mDebugTraces.erase(index.mDebugMarker);
                    assert(count == 1);
                    index.mDebugMarker = 0;
#endif
                    break;
                }

                assert(entry.mOp == ERASE || entry.mOp == SWAP_ERASE);
                if (index.mIndex == entry.mArg1) {
                    index.reset();
#if ENABLE_MEMTRACKING
                    std::lock_guard lock { mDebugMutex };
                    auto count = mDebugTraces.erase(index.mDebugMarker);
                    assert(count == 1);
                    index.mDebugMarker = 0;
#endif
                    break;
                }
                if (entry.mOp == SWAP_ERASE) {
                    if (index.mIndex == entry.mArg2) {
                        index.mIndex = entry.mArg1;
                    }
                } else {
                    assert(entry.mOp == ERASE);
                }
                index.mGeneration = (index.mGeneration + 1) % wrap;
            }

            if (index)
                incRef(index.mGeneration);
            decRef(oldGen);

#if ENABLE_MEMTRACKING
            if (index) {
                std::lock_guard lock { mDebugMutex };
                mDebugTraces.at(index.mDebugMarker).first = mGeneration;
            }
#endif
        }
        return oldIndex;
    }

protected:
    void onSwapErase(uint32_t index, uint32_t swapped)
    {
        uint16_t gen = mGeneration.load();
        uint16_t nextGen = (gen + 1) % mHistory.size();
        if (mHistory[nextGen].mRefCount != 0)
            throw "Ring buffer is full!";
        mHistory[gen].mOp = SWAP_ERASE;
        mHistory[gen].mArg1 = index;
        mHistory[gen].mArg2 = swapped;
        if (!mGeneration.compare_exchange_strong(gen, nextGen))
            throw "Concurrent writes!!";
    }

    void onErase(uint32_t index)
    {
        uint16_t gen = mGeneration.load();
        uint16_t nextGen = (gen + 1) % mHistory.size();
        if (mHistory[nextGen].mRefCount != 0)
            throw "Ring buffer is full!";
        mHistory[gen].mOp = ERASE;
        mHistory[gen].mArg1 = index;
        if (!mGeneration.compare_exchange_strong(gen, nextGen))
            throw "Concurrent writes!!";
    }

    void onClear()
    {

        uint16_t gen = mGeneration.load();
        uint16_t nextGen = (gen + 1) % mHistory.size();
        if (mHistory[nextGen].mRefCount != 0)
            throw "Ring buffer is full!";
        mHistory[gen].mOp = RESET;
        if (!mGeneration.compare_exchange_strong(gen, nextGen))
            throw "Concurrent writes!!";
    }

    uint32_t getIndex(const GenerationContainerIndex &index) const
    {
        assert(index);
        return index.mIndex;
    }

    uint32_t getIndex(const GenerationContainerIndex &index, uint32_t end) const
    {
        return index ? uint32_t { index.mIndex } : end;
    }

private:
    void incRef(uint16_t index) const
    {
        ++mHistory[index].mRefCount;
    }

    void decRef(uint16_t index) const
    {
        --mHistory[index].mRefCount;
    }

    struct Entry {
        mutable std::atomic<uint16_t> mRefCount = 0;
        ContainerEvent mOp;
        uint32_t mArg1, mArg2;
    };

    std::vector<Entry> mHistory;
    std::atomic<uint16_t> mGeneration = 0;

#if ENABLE_MEMTRACKING
    mutable uint32_t mDebugCounter = 0;
    mutable std::unordered_map<uint32_t, std::pair<uint16_t, Debug::StackTrace<8>>> mDebugTraces;
    mutable std::mutex mDebugMutex;
#endif
};

template <typename C>
struct GenerationContainer : GenerationContainerBase, protected C {

    using value_type = typename C::value_type;
    using reference = typename C::reference;
    using const_reference = typename C::const_reference;
    using pointer = typename C::pointer;
    using const_pointer = typename C::const_pointer;

    using GenerationContainerBase::GenerationContainerBase;

    struct iterator {

        using iterator_category = typename C::iterator::iterator_category;
        using value_type = typename C::iterator::value_type;
        using difference_type = typename C::iterator::difference_type;
        using pointer = typename C::iterator::pointer;
        using reference = typename C::iterator::reference;

        iterator() = default;

        iterator(GenerationContainerIndex index, GenerationContainer<C> *vector)
            : mIndex(std::move(index))
            , mVector(vector)
        {
        }

        iterator(const iterator &other)
            : mIndex(other.mIndex ? other.mVector->copy(other.mIndex) : GenerationContainerIndex {})
            , mVector(other.mVector)
        {
        }

        iterator(iterator &&) = default;

        ~iterator()
        {
            mVector->reset(mIndex);
        }

        reference operator*() const
        {
            return (*mVector)[mIndex];
        }

        pointer operator->() const
        {
            return &(*mVector)[mIndex];
        }

        iterator &operator=(const iterator &other)
        {
            if (mVector)
                mVector->reset(mIndex);
            mVector = other.mVector;
            if (mVector)
                mIndex = mVector->copy(other.mIndex);
            return *this;
        }

        iterator &operator++()
        {
            mVector->increment(mIndex, mVector->size());
            return *this;
        }

        bool operator==(const iterator &other) const
        {
            assert(mVector == other.mVector || !mVector || !other.mVector);
            if (mVector)
                mVector->update(mIndex);
            if (other.mVector)
                other.mVector->update(other.mIndex);
            return mIndex == other.mIndex;
        }

        bool operator!=(const iterator &other) const
        {
            assert(mVector == other.mVector || !mVector || !other.mVector);
            if (mVector)
                mVector->update(mIndex);
            if (other.mVector)
                other.mVector->update(other.mIndex);
            return mIndex != other.mIndex;
        }

        ptrdiff_t operator-(const iterator &other) const
        {
            assert(mVector == other.mVector);
            mVector->update(mIndex);
            mVector->update(other.mIndex);
            return mVector->getIndex(mIndex, mVector->size()) - mVector->getIndex(other.mIndex, mVector->size());
        }

        iterator &operator+=(ptrdiff_t diff)
        {
            mVector->increment(mIndex, mVector->size(), diff);
            return *this;
        }

        iterator operator+(ptrdiff_t diff) const
        {
            GenerationContainerIndex index = mVector->copy(mIndex);
            mVector->increment(index, mVector->size(), diff);
            return { std::move(index), mVector };
        }

        void update() const
        {
            if (mVector)
                mVector->update(mIndex);
        }

        GenerationContainerIndex copyIndex() const
        {
            return mVector->copy(mIndex);
        }

        uint32_t index() const
        {
            return mVector->getIndex(mIndex);
        }

        bool valid() const
        {
            update();
            return mIndex.operator bool();
        }

        mutable GenerationContainerIndex mIndex;
        GenerationContainer<C> *mVector = nullptr;
    };

    struct const_iterator {

        using iterator_category = typename C::const_iterator::iterator_category;
        using value_type = typename C::const_iterator::value_type;
        using difference_type = typename C::const_iterator::difference_type;
        using pointer = typename C::const_iterator::pointer;
        using reference = typename C::const_iterator::reference;

        const_iterator() = default;

        const_iterator(const const_iterator &other)
            : mIndex(other.mVector->copy(other.mIndex))
            , mVector(other.mVector)
        {
        }

        const_iterator(const iterator &other)
            : mIndex(other.mVector->copy(other.mIndex))
            , mVector(other.mVector)
        {
        }

        const_iterator(const_iterator &&) = default;

        const_iterator(GenerationContainerIndex index, const GenerationContainer<C> *vector)
            : mIndex(std::move(index))
            , mVector(vector)
        {
        }

        ~const_iterator()
        {
            mVector->reset(mIndex);
        }

        reference operator *() const
        {
            return (*mVector)[mIndex];
        }

        pointer operator->() const
        {
            return &(*mVector)[mIndex];
        }

        const_iterator &operator++()
        {
            mVector->increment(mIndex, mVector->size());
            return *this;
        }

        bool operator!=(const const_iterator &other)
        {
            assert(mVector == other.mVector);
            mVector->update(mIndex);
            mVector->update(other.mIndex);
            return mIndex != other.mIndex;
        }

        ptrdiff_t operator-(const const_iterator &other) const
        {
            assert(mVector == other.mVector);
            mVector->update(mIndex);
            mVector->update(other.mIndex);
            return mVector->getIndex(mIndex, mVector->size()) - mVector->getIndex(other.mIndex, mVector->size());
        }

        const_iterator operator+(ptrdiff_t diff) const
        {
            GenerationContainerIndex index = mVector->copy(mIndex);
            mVector->increment(index, mVector->size(), diff);
            return { std::move(index), mVector };
        }

        GenerationContainerIndex copyIndex() const
        {
            return mVector->copy(mIndex);
        }

        mutable GenerationContainerIndex mIndex;
        const GenerationContainer<C> *mVector;
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <typename... Args>
    iterator emplace(Args &&... args)
    {
        pointer t = &C::emplace_back(std::forward<Args>(args)...);
        uint32_t index = t - &C::front();
        return { generate(index), this };
    }

    iterator erase(const iterator &it)
    {
        return erase(it.mIndex);
    }

    iterator erase(GenerationContainerIndex &it)
    {
        update(it);

        uint32_t size = C::size();
        uint32_t index = getIndex(it);

        assert(index < size);

        if (index != size - 1) {
            using std::swap;
            swap(C::operator[](index), C::operator[](size - 1));
            onSwapErase(index, size - 1);
        } else {
            onErase(index);
        }
        C::pop_back();
        reset(it);
        return iterator { generate(index), this };
    }

    void clear()
    {
        onClear();
        C::clear();
    }

    size_t size() const
    {
        return C::size();
    }

    bool empty() const
    {
        return C::empty();
    }

    reference front()
    {
        return C::front();
    }

    reference operator[](size_t i) {
        return C::operator[](i);
    }

    const_reference operator[](size_t i) const
    {
        return C::operator[](i);
    }

    reference operator[](GenerationContainerIndex &index)
    {
        update(index);
        assert(index);
        return C::operator[](getIndex(index));
    }

    const_reference operator[](GenerationContainerIndex &index) const
    {
        update(index);
        assert(index);
        return C::operator[](getIndex(index));
    }

    reference at(size_t i)
    {
        return C::at(i);
    }

    const_reference at(size_t i) const
    {
        return C::at(i);
    }

    reference at(GenerationContainerIndex &index)
    {
        update(index);
        assert(index);
        return C::at(getIndex(index));
    }

    const_reference at(GenerationContainerIndex &index) const
    {
        update(index);
        assert(index);
        return C::at(getIndex(index));
    }

    iterator begin()
    {
        if (C::empty())
            return end();
        return { generate(0), this };
    }

    iterator end()
    {
        return { {}, this };
    }

    const_iterator begin() const
    {
        if (C::empty())
            return end();
        return { generate(0), this };
    }

    const_iterator end() const
    {
        return { {}, this };
    }

    reverse_iterator rbegin()
    {
        if (C::empty())
            return rend();
        return { generate(size() - 1), this };
    }

    reverse_iterator rend()
    {
        return { {}, this };
    }

    const_reverse_iterator rbegin() const
    {
        if (C::empty())
            return rend();
        return { generate(size() - 1), this };
    }

    const_reverse_iterator rend() const
    {
        return { {}, this };
    }

    iterator fetch(GenerationContainerIndex& index) {        
        return { copy(index), this };
    }

};

template <typename T>
using GenerationVector = GenerationContainer<std::vector<T>>;


template <typename C>
struct underlying_container<GenerationContainer<C>> {
    typedef C type;
};

template <typename C>
struct container_traits<GenerationContainer<C>> {
    static constexpr const bool sorted = true;
    static constexpr const bool has_dependent_handle = true;
    static constexpr const bool remove_invalidates_handles = true;
    static constexpr const bool is_fixed_size = false;

    typedef typename GenerationContainer<C>::iterator iterator;
    typedef typename GenerationContainer<C>::const_iterator const_iterator;

    typedef IndexType<size_t> handle;
    typedef IndexType<size_t> const_handle;
    typedef IndexType<size_t> position_handle;
    typedef IndexType<size_t> const_position_handle;

    static_assert(sizeof(position_handle) <= sizeof(void *));

    static position_handle toPositionHandle(GenerationContainer<C> &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static handle toHandle(GenerationContainer<C> &c, const position_handle &handle)
    {
        return handle;
    }

    static handle toHandle(GenerationContainer<C> &c, const iterator &it)
    {
        return std::distance(c.begin(), it);
    }

    static void revalidateHandleAfterInsert(position_handle &handle, const GenerationContainer<C> &c, const const_iterator &it)
    {
        size_t item = std::distance(c.begin(), it);
        if (item <= handle)
            ++handle;
    }

    static void revalidateHandleAfterRemove(position_handle &handle, const GenerationContainer<C> &c, const const_iterator &it, bool wasIn, size_t count = 1)
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

    static iterator toIterator(GenerationContainer<C> &c, const position_handle &handle)
    {
        return c.begin() + handle;
    }

    static const_iterator toIterator(const GenerationContainer<C> &c, const const_position_handle &handle)
    {
        return c.begin() + handle;
    }

    static position_handle next(const position_handle &handle)
    {
        return handle + 1;
    }
};

template <typename C, typename C2>
struct container_api_impl<C, GenerationContainer<C2>> : container_api_impl<C, C2> {

    using container_api_impl<C, C2>::container_api_impl;

    using C::operator=;

    using value_type = typename C::value_type;

    /*void resize(size_t size)
            {
                C::resize(size);
            }*/

    /*void remove(const value_type &item)
    {
        for (typename C::const_iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }*/

    /*decltype(auto) push_back(const value_type &item)
    {
        return then(emplace_back(item), [](auto &&it) -> decltype(auto) {
            return *it;
        });
    }*/

    /*decltype(auto) push_back(value_type &&item)
    {
        return then(emplace_back(std::move(item)), [](auto &&it) -> decltype(auto) {
            return *it;
        });
    }*/

    /*template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&... args)
    {
        return this->emplace(this->end(), std::forward<_Ty>(args)...);
    }*/

    using C::at;
    using C::operator[];
    using C::copy;
    using C::reset;
    using C::update;
};

}