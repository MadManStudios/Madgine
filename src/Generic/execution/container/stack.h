#pragma once

namespace Engine {
namespace Execution {

    template <typename T>
    struct ConnectionStack {

        ConnectionStack() = default;
        ConnectionStack(const ConnectionStack &) = delete;
        ConnectionStack(ConnectionStack &&other)
        {
            std::unique_lock lock { other.mMutex };
            mHead = other.mHead.exchange(nullptr);
        }

        ~ConnectionStack()
        {
            clear();
        }

        bool push(T *item)
        {
            std::unique_lock lock { mMutex };
            assert(!item->mNext);
            T *previous = mHead;
            item->mNext = previous;
            while (!mHead.compare_exchange_weak(previous, item))
                item->mNext = previous;
            return !previous;
        }

        T *pop()
        {
            std::unique_lock lock { mMutex };
            T *head = nullptr;
            T *newHead = nullptr;
            while (!mHead.compare_exchange_weak(head, newHead)) {
                if (head) {
                    newHead = head->mNext;
                } else {
                    newHead = nullptr;
                }
            }
            return head;
        }

        bool empty() const
        {
            std::unique_lock lock { mMutex };
            return !mHead;
        }

        bool extract(T *con)
        {
            assert(con);

            std::lock_guard guard { mMutex };

            std::atomic<T *> *next = &mHead;
            T *current = mHead;
            while (current) {
                if (current == con) {
                    if (next->compare_exchange_strong(current, current->mNext.load())) {
                        current->mNext = nullptr;
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    next = &current->mNext;
                    current = *next;
                }
            }
            return false;
        }

        void clear()
        {
            T *current;
            {
                std::lock_guard guard { mMutex };
                current = mHead.exchange(nullptr);
            }
            while (current) {
                T *next = current->mNext.exchange(nullptr);
                current->set_done();
                current = next;
            }
        }

        std::mutex &mutex() const
        {
            return mMutex;
        }

        struct iterator {
            T *mCurrent;

            operator T *() const
            {
                return mCurrent;
            }

            T &operator*() const
            {
                return mCurrent;
            }

            iterator &operator++()
            {
                mCurrent = mCurrent->mNext;
                return *this;
            }

            constexpr auto operator<=>(const iterator &) const = default;
        };

        iterator begin()
        {
            return { mHead };
        }

        iterator end()
        {
            return { nullptr };
        }

    private:
        std::atomic<T *> mHead = nullptr;
        mutable std::mutex mMutex;
    };

}
}