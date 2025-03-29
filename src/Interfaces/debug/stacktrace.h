#pragma once

#ifndef NDEBUG

#include "traceback.h"

namespace Engine {
namespace Debug {

    using FullStackTrace = std::pmr::vector<TraceBack>;

    INTERFACES_EXPORT void getStackTrace(size_t skip, void **buffer, size_t size);
    INTERFACES_EXPORT FullStackTrace resolveSymbols(void *const *data, size_t size);

    template <size_t S>
    struct StackTrace {

        StackTrace() = default;

        StackTrace(const StackTrace<S> &other)
            : mTrace(other.mTrace)
        {
        }

        FullStackTrace calculateReadable(size_t count = S) const
        {
            size_t itemCount;
            for (itemCount = 0; itemCount < S && mTrace[itemCount] != nullptr; ++itemCount)
                ;
            return resolveSymbols(mTrace.data(), itemCount);
        }

        template <size_t U>
        StackTrace<U> subset(size_t offset = 0) const {
            static_assert(U <= S);
            assert(U + offset <= S);
            StackTrace<U> result;
            std::copy(mTrace.begin() + offset, mTrace.begin() + offset + U, result.begin());
            return result;
        }

        static StackTrace<S> getCurrent(size_t skip)
        {
            StackTrace<S> result;
            getStackTrace(skip + 1, result.mTrace.data(), S);
            return result;
        }

        bool operator==(const StackTrace<S> &other) const
        {
            for (size_t i = 0; i < S; ++i) {
                if (mTrace[i] != other.mTrace[i])
                    return false;
                if (mTrace[i] == nullptr)
                    break;
            }
            return true;
        }

        struct iterator {
            bool operator!=(const iterator& other) {
                return mCurrent != other.mCurrent;
            }

            void operator++() {
                assert(mCurrent < mEnd);
                ++mCurrent;
                if (mCurrent != mEnd && !*mCurrent)
                    mCurrent = mEnd;
            }

            void* &operator*() {
                return *mCurrent;
            }

            void **mCurrent, **mEnd;
        };

        iterator begin() {
            return { mTrace.data(), mTrace.data() + S };
        }

        iterator end()
        {
            return { mTrace.data() + S, mTrace.data() + S };
        }

    private:
        friend struct std::hash<StackTrace<S>>;

        std::array<void *, S> mTrace;
    };

}
}

namespace std {
template <size_t S>
struct hash<Engine::Debug::StackTrace<S>> {
    size_t operator()(const Engine::Debug::StackTrace<S> &stacktrace) const
    {
        size_t hash = 0;
        for (int i = 0; i < S && stacktrace.mTrace[i]; ++i) {
            if (stacktrace.mTrace[i] == nullptr)
                break;
            hash = reinterpret_cast<size_t>(stacktrace.mTrace[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
};

template <>
struct hash<Engine::Debug::FullStackTrace> {
    size_t operator()(const Engine::Debug::FullStackTrace &stacktrace) const
    {
        std::hash<Engine::Debug::TraceBack> helper;
        size_t hash = 0;
        for (const Engine::Debug::TraceBack &t : stacktrace) {
            hash = helper(t) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
};
}

#endif