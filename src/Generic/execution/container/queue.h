#pragma once


namespace Engine {
namespace Execution {

    template <typename T>
    struct ConnectionQueue {

        ConnectionQueue() = default;
        ConnectionQueue(const ConnectionQueue &) = delete;
        ConnectionQueue(ConnectionQueue &&) = delete;

        ~ConnectionQueue()
        {
            clear();
        }

        bool push(T *item)
        {
            std::unique_lock lock { mMutex };
            T *previous = mTail;
            while (!mTail.compare_exchange_weak(previous, item))
                ;
            bool wasFirst = previous == nullptr;
            if (previous) {
                previous->mNext = item;
            } else {
                T *expected = nullptr;
                bool result = mHead.compare_exchange_strong(expected, item);
                assert(result);
            }
            return wasFirst;
        }

        T *pop()
        {
            std::unique_lock lock { mMutex };
            T *head = nullptr;
            T *newHead = nullptr;
            while (!mHead.compare_exchange_weak(head, newHead)) {
                if (head) {
                    if (head != mTail)
                        while (!head->mNext)
                            ;
                    newHead = head->mNext;
                } else {
                    newHead = nullptr;
                }
            }
            if (head == mTail)
                mTail = nullptr;
            return head;
        }

        bool empty() const
        {
            std::unique_lock lock { mMutex };
            return !mHead;
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
                current->cancel();
                current = next;
            }
        }

        std::mutex &mutex() const
        {
            return mMutex;
        }

    private:
        std::atomic<T *> mHead = nullptr;
        std::atomic<T *> mTail = nullptr;
        mutable std::mutex mMutex;
    };

}
}