#pragma once

#include "Generic/any.h"

namespace Engine {
namespace Threading {

    template <typename T, typename Storage>
    struct Global {

        template <typename... Args>
        Global(Args &&... args)
        {
            std::tuple<Args...> tuple {
                std::forward<Args>(args)...
            };
            auto lambda = [=]() { return TupleUnpacker::constructExpand<Any>(Any::inplace<T>, tuple); };

            if constexpr (std::is_trivially_copyable_v<T>) {
                mIndex = Storage::registerLocalBssVariable(lambda);
            } else {
                mIndex = Storage::registerLocalObjectVariable(lambda);
            }
        }

        ~Global()
        {
            if constexpr (std::is_trivially_copyable_v<T>) {
                Storage::unregisterLocalBssVariable(mIndex);
            } else {
                Storage::unregisterLocalObjectVariable(mIndex);
            }
            mIndex = INVALID;
        }

        T *operator->() { return std::addressof(data()); }

        T &operator*() { return data(); }

        operator T &() { return data(); }

        T *operator&() { return std::addressof(data()); }

        Global<T, Storage> &operator=(const Global<T, Storage> &other)
        {
            data() = other.data();
            return *this;
        }

        Global<T, Storage> &operator=(T &&t)
        {
            data() = std::move(t);
            return *this;
        }

        Global<T, Storage> &operator=(const T &t)
        {
            data() = t;
            return *this;
        }

        T &data()
        {
            assert(valid());
            return Storage::localVariable(mIndex).template as<T>();
        }

        const T &data() const
        {
            assert(valid());
            return Storage::localVariable(mIndex).template as<T>();
        }
                
        bool valid() const
        {
            return mIndex != INVALID;
        }

    private:
        static constexpr int INVALID = std::numeric_limits<int>::min();

        int mIndex;
    };

    template <typename T, typename Storage>
    struct Global<T *, Storage> {

        Global(T *initial = nullptr)
        {
            mIndex = Storage::registerLocalBssVariable([=]() { return Any { (T *)initial }; });
        }

        ~Global()
        {
            Storage::unregisterLocalBssVariable(mIndex);
            mIndex = INVALID;
        }

        T *operator->() { return data(); }

        T &operator*() { return *data(); }

        operator T *&() { return data(); }

        T **operator&() { return &data(); }

        Global<T *, Storage> &operator=(const Global<T *, Storage> &other)
        {
            data() = other.data();
            return *this;
        }

        Global<T *, Storage> &operator=(T *t)
        {
            data() = t;
            return *this;
        }

        T *&data()
        {
            assert(valid());
            return Storage::localVariable(mIndex).template as<T *>();
        }

        T *const &data() const
        {
            assert(valid());
            return Storage::localVariable(mIndex).template as<T *>();
        }

        bool valid() const
        {
            return mIndex != INVALID;
        }

    private:
        static constexpr int INVALID = std::numeric_limits<int>::min();

        int mIndex;
    };

} // namespace Threading
} // namespace Engine
