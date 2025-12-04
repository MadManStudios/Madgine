#pragma once

namespace Engine {

template <typename T, typename Holder = std::unique_ptr<T>>
struct HeapObject {

    template <typename... Args>
    HeapObject(Args &&...args)
        : mObject(std::make_unique<T>(std::forward<Args>(args)...))
    {
    }

    HeapObject(Holder holder)
        : mObject(std::move(holder))
    {
    }

    HeapObject(const HeapObject<T, Holder> &other)
        : mObject(std::make_unique<T>(*other))
    {
    }

    HeapObject(HeapObject<T, Holder> &&) = default;

    HeapObject<T, Holder> &operator=(const HeapObject<T, Holder> &other)
    {
        *mObject = *other;
        return *this;
    }

    bool operator==(const HeapObject<T, Holder> &other) const
    {
        return *mObject == *other;
    }

    operator T &()
    {
        return *mObject;
    }

    operator const T &() const
    {
        return *mObject;
    }

    T &operator*()
    {
        return *mObject;
    }

    const T &operator*() const
    {
        return *mObject;
    }

    T *operator->()
    {
        return mObject.get();
    }

    const T *operator->() const
    {
        return mObject.get();
    }

private:
    Holder mObject;
};

}