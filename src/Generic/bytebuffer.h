#pragma once

#include "any.h"

namespace Engine {

struct ByteBufferDataAccessor {
    template <typename T>
    auto operator()(const std::vector<T> &v) { return v.data(); }

    template <typename T>
    auto operator()(std::vector<T> &v) { return v.data(); }

    template <typename T, typename D>
    auto operator()(const std::unique_ptr<T, D> &p) { return p.get(); }

    template <typename T, size_t Size>
    const T *operator()(const T (&a)[Size]) { return a; }
};
struct ByteBufferSizeAccessor {
    template <typename T>
    size_t operator()(const std::vector<T> &v) { return v.size() * sizeof(T); }

    template <typename T, size_t Size>
    size_t operator()(const T (&)[Size]) { return Size * sizeof(T); }
};

/**
 * @brief 
 * @tparam Data 
*/
template <typename Data>
struct ByteBufferImpl {

    ByteBufferImpl() = default;

    template <typename T>
    requires(requires(T &t) {
        ByteBufferSizeAccessor {}(t);
        ByteBufferDataAccessor {}(t);
    })
        ByteBufferImpl(T &&t)
        : mKeep(std::forward<T>(t))
        , mSize(ByteBufferSizeAccessor {}(mKeep.as<T>()))
        , mData(ByteBufferDataAccessor {}(mKeep.as<T>()))
    {
    }

    template <typename T>
    requires(requires(T &t) {        
        ByteBufferDataAccessor {}(t);
    })
        ByteBufferImpl(T &&t, size_t size)
        : mKeep(std::forward<T>(t))
        , mSize(size)
        , mData(ByteBufferDataAccessor {}(mKeep.as<T>()))
    {
    }

    template <typename T>
    requires(!Pointer<std::remove_reference_t<T>>)
        ByteBufferImpl(T &&t, size_t size, Data *data)
        : mKeep(std::forward<T>(t))
        , mSize(size)
        , mData(data)
    {
    }

    ByteBufferImpl(Data *data, size_t size)
        : mSize(size)
        , mData(data)
    {
    }

    void clear()
    {
        mSize = 0;
        mData = nullptr;
        mKeep = {};
    }

    template <typename T>
    ByteBufferImpl<T> cast() &&
    {
        return {
            std::move(mKeep),
            mSize,
            static_cast<std::remove_extent_t<T> *>(mData)
        };
    }

    Data *begin() const
    {
        return mData;
    }

    Data *end() const
    {
        if constexpr (std::same_as<std::remove_const_t<Data>, void>)
            return static_cast<const std::byte *>(mData) + mSize;
        else
            return mData + mSize;
    }

    Data *operator->() const
    {
        return mData;
    }

    bool operator==(const ByteBufferImpl &other) const
    {
        if (mSize != other.mSize)
            return false;
        return std::memcmp(mData, other.mData, mSize) == 0;
    }

private:
    Any mKeep;

public:
    size_t mSize = 0;
    Data *mData = nullptr;
};

template <typename Data>
struct ByteBufferImpl<Data[]> : ByteBufferImpl<Data> {

    using ByteBufferImpl<Data>::ByteBufferImpl;

    Data &operator[](size_t index)
    {
        return this->mData[index];
    }

    size_t elementCount() const
    {
        assert(this->mSize % sizeof(Data) == 0);
        return this->mSize / sizeof(Data);
    }
};

using ByteBuffer = ByteBufferImpl<const void>;
using WritableByteBuffer = ByteBufferImpl<void>;

template <typename T>
ByteBufferImpl(std::vector<T>) -> ByteBufferImpl<T[]>;

}