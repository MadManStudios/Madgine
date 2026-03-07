#pragma once

namespace Engine {

struct TypedPtr {

    TypedPtr() = default;

    template <typename T>
    TypedPtr(T *t)
        : mType(&typeid(T))
        , mValue(t)
    {
    }

    explicit operator bool() const
    {
        return mValue;
    }

    template <typename T>
    bool operator==(T *t) const
    {
        return t == mValue && *mType == typeid(T);
    }

    TypedPtr &operator=(std::nullptr_t)
    {
        mType = nullptr;
        mValue = nullptr;
        return *this;
    }

    template <typename T>
    TypedPtr &operator=(T *t)
    {
        mType = &typeid(T);
        mValue = t;
        return *this;
    }

    const std::type_info &type() const
    {
        return *mType;
    }

    const void *ptr() const
    {
        return mValue;
    }

    template <typename T>
    T *as() const
    {
        if (!mValue)
            return nullptr;
        if (*mType != typeid(T)) {
            return nullptr;
        }
        return static_cast<T *>(mValue);
    }

    bool operator==(const TypedPtr &other) const
    {
        return mType == other.mType && mValue == other.mValue;
    }

private:
    const std::type_info *mType = nullptr;
    const void *mValue = nullptr;
};

}