#pragma once

#if __cpp_lib_atomic_shared_ptr < 201711

namespace std {

template <typename T>
struct atomic<std::shared_ptr<T>> {

    std::shared_ptr<T> load() const
    {
        return atomic_load(&mPtr);
    }

    bool compare_exchange_strong(std::shared_ptr<T> &expected, std::shared_ptr<T> desired)
    {
        return atomic_compare_exchange_strong(&mPtr, &expected, std::move(desired));
    }

    void store(std::shared_ptr<T> ptr)
    {
        atomic_store(&mPtr, std::move(ptr));
    }

private:
    std::shared_ptr<T> mPtr;
};

}

#endif