#pragma once

#include "any.h"

namespace Engine {
namespace Memory {

    namespace __generic_impl__ {

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

    }

    /**
     * @brief
     * @tparam Data
     */
    template <typename Data>
    struct TypedByteBuffer {

        TypedByteBuffer() = default;

        template <typename T>
            requires(requires(T &t) {
                __generic_impl__::ByteBufferSizeAccessor {}(t);
                __generic_impl__::ByteBufferDataAccessor {}(t);
            })
        TypedByteBuffer(T &&t)
            : mKeep(std::forward<T>(t))
            , mSize(__generic_impl__::ByteBufferSizeAccessor {}(mKeep.as<T>()))
            , mData(__generic_impl__::ByteBufferDataAccessor {}(mKeep.as<T>()))
        {
        }

        template <typename T>
            requires(requires(T &t) {
                __generic_impl__::ByteBufferDataAccessor {}(t);
            })
        TypedByteBuffer(T &&t, size_t size)
            : mKeep(std::forward<T>(t))
            , mSize(size)
            , mData(__generic_impl__::ByteBufferDataAccessor {}(mKeep.as<T>()))
        {
        }

        template <typename T>
            requires(!Concepts::Pointer<std::remove_reference_t<T>>)
        TypedByteBuffer(T &&t, size_t size, Data *data)
            : mKeep(std::forward<T>(t))
            , mSize(size)
            , mData(data)
        {
        }

        TypedByteBuffer(Data *data, size_t size)
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
        TypedByteBuffer<T> cast() &&
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

        patch_void_t<Data> &operator*() const
            requires(!std::is_void_v<Data>)
        {
            return *mData;
        }

        bool operator==(const TypedByteBuffer &other) const
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
    struct TypedByteBuffer<Data[]> : TypedByteBuffer<Data> {

        using TypedByteBuffer<Data>::TypedByteBuffer;

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

    template <typename T>
    TypedByteBuffer(std::vector<T>) -> TypedByteBuffer<T[]>;

    using ByteBuffer = TypedByteBuffer<const void>;
    using WritableByteBuffer = TypedByteBuffer<void>;

}
}