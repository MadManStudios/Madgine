#pragma once

namespace Engine {

template <typename T, size_t Dim, size_t Storage = Dim>
struct AreaView {

    AreaView(T *buffer, const std::array<size_t, Storage> &sizes)
        : mBuffer(buffer)
        , mSizes(sizes)
        , mFullSizes(sizes)
    {
        mFlipped.fill(false);
        for (size_t i = 0; i < Dim; ++i)
            mAxisMapping[i] = i;
    }

    AreaView(T *buffer, const std::array<size_t, Storage> &sizes, const std::array<size_t, Storage> &fullSizes, const std::array<bool, Storage> &flipped, const std::array<size_t, Storage> &axisMapping)
        : mBuffer(buffer)
        , mSizes(sizes)
        , mFullSizes(fullSizes)
        , mFlipped(flipped)
        , mAxisMapping(axisMapping)
    {
    }

    AreaView<T, Dim, Storage> subArea(const std::array<size_t, Storage> &topLeft, const std::array<size_t, Storage> &sizes)
    {
        size_t acc = 0;
        for (int i = Dim - 1; i >= 0; --i) {
            size_t axis = mAxisMapping[i];
            assert(topLeft[i] <= mSizes[axis]);
            acc *= mFullSizes[axis];
            if (!mFlipped[axis]) {
                acc += topLeft[i];
            } else {
                acc += (mSizes[axis] - topLeft[i] - sizes[i]);
            }
        }
        return { mBuffer + acc, sizes, mFullSizes, mFlipped, mAxisMapping };
    }

    decltype(auto) operator[](size_t i) const
    {
        size_t axis = mAxisMapping[Dim - 1];
        assert(i < mSizes[axis]);
        if (mFlipped[axis])
            i = mSizes[axis] - 1 - i;
        size_t fullSize = 1;
        for (size_t j = 0; j < axis; ++j) {
            fullSize *= mFullSizes[j];
        }
        T *ptr = mBuffer + i * fullSize;
        if constexpr (Dim == 1) {
            return *ptr;
        } else {
            return AreaView<T, Dim - 1, Storage> { ptr, mSizes, mFullSizes, mFlipped, mAxisMapping };
        }
    }

    T &at(const std::array<size_t, Dim> &indices)
    {
        return mBuffer[index(indices)];
    }

    const T &at(const std::array<size_t, Dim> &indices) const
    {
        return mBuffer[index(indices)];
    }

    void flip(size_t dim)
    {
        mFlipped[mAxisMapping[dim]] = !mFlipped[mAxisMapping[dim]];
    }

    void swapAxis(size_t a1, size_t a2)
    {
        std::swap(mAxisMapping[a1], mAxisMapping[a2]);
    }

    size_t index(const std::array<size_t, Dim> &indices) const
    {
        size_t acc = 0;
        for (int i = Dim - 1; i >= 0; --i) {
            size_t reverseAxis = std::ranges::find(mAxisMapping, i) - mAxisMapping.begin();
            acc *= mFullSizes[i];
            if (reverseAxis < Dim) {
                assert(indices[reverseAxis] < mSizes[i]);
                if (!mFlipped[i]) {
                    acc += indices[reverseAxis];
                } else {
                    acc += (mSizes[i] - 1 - indices[reverseAxis]);
                }
            }
        }
        return acc;
    }

    template <bool isConst>
    struct IteratorImpl {

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const_if<isConst, T>;
        using difference_type = ptrdiff_t;
        using pointer = void;
        using reference = const_if<isConst, T> &;

        using ViewType = const_if<isConst, AreaView<T, Dim, Storage>>;

        IteratorImpl() = default;

        IteratorImpl(ViewType *view)
            : mView(view)
        {
            mIndices.fill(0);
        }

        IteratorImpl(ViewType *view, const std::array<size_t, Dim> &indices)
            : mView(view)
            , mIndices(indices)
        {
        }

        reference operator*() const
        {
            return mView->at(mIndices);
        }

        IteratorImpl &operator++()
        {
            ++mIndices[0];
            validateIncrement();
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
            validateDecrement();
            --mIndices[0];
            return *this;
        }

        IteratorImpl operator--(int)
        {
            IteratorImpl copy = *this;
            --*this;
            return copy;
        }

        bool operator==(const IteratorImpl &other) const
        {
            return mView == other.mView && mIndices == other.mIndices;
        }

        bool operator!=(const IteratorImpl &other) const
        {
            return !(*this == other);
        }

    private:
        void validateIncrement()
        {
            if constexpr (Dim == 1) {
                assert(mIndices[0] <= mView->mSizes[mView->mAxisMapping[0]]);
            } else {
                assert(mIndices[Dim - 1] < mView->mSizes[mView->mAxisMapping[Dim - 1]]);
                for (size_t i = 0; i < Dim - 1; ++i) {
                    size_t axis = mView->mAxisMapping[i];
                    assert(mIndices[i] <= mView->mSizes[axis]);
                    if (mIndices[i] == mView->mSizes[axis]) {
                        mIndices[i] = 0;
                        ++mIndices[i + 1];
                    } else {
                        break;
                    }
                }
            }
        }

        void validateDecrement()
        {
            if constexpr (Dim == 1) {
                assert(mIndices[0] > 0);
            } else {
                for (size_t i = 0; i < Dim - 1; ++i) {
                    size_t axis = mView->mAxisMapping[i];
                    if (mIndices[i] == 0) {
                        assert(mIndices[i + 1] > 0);
                        mIndices[i] = mView->mSizes[axis];
                        --mIndices[i + 1];
                    } else {
                        break;
                    }
                }
            }
        }

    private:
        ViewType *mView = nullptr;
        std::array<size_t, Dim> mIndices;
    };

    using iterator = IteratorImpl<false>;
    using const_iterator = IteratorImpl<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin()
    {
        return { this };
    }

    iterator end()
    {
        std::array<size_t, Dim> endSizes;
        endSizes.fill(0);
        endSizes[Dim - 1] = mSizes[mAxisMapping[Dim - 1]];
        return { this, endSizes };
    }

    const_iterator begin() const
    {
        return { this };
    }

    const_iterator end() const
    {
        std::array<size_t, Dim> endSizes;
        endSizes.fill(0);
        endSizes[Dim - 1] = mSizes[mAxisMapping[Dim - 1]];
        return { this, endSizes };
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator { end() };
    }

    reverse_iterator rend()
    {
        return reverse_iterator { begin() };
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator { end() };
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator { begin() };
    }

private:
    T *mBuffer;
    std::array<size_t, Storage> mSizes;
    std::array<size_t, Storage> mFullSizes;
    std::array<bool, Storage> mFlipped;
    std::array<size_t, Storage> mAxisMapping;
};

}