#pragma once

namespace Engine {

struct CallerHierarchyBase {
    const std::type_info &mType;
    const CallerHierarchyBase *mParent = nullptr;
};

template <typename T>
struct CallerHierarchy : CallerHierarchyBase {
    CallerHierarchy(T t, const CallerHierarchyBase *parent = nullptr)
        : CallerHierarchyBase { typeid(T), parent }
        , mData(std::move(t))
    {
    }

    template <std::convertible_to<T> U>
    CallerHierarchy(const CallerHierarchy<U> &other)
        : CallerHierarchyBase { other }
        , mData(other.mData)
    {
    }

    T mData;
};

struct CallerHierarchyBasePtr {
    const CallerHierarchyBase *mPtr = nullptr;

    CallerHierarchyBasePtr() = default;

    template <typename T>
    CallerHierarchyBasePtr(const CallerHierarchy<T> &t)
        : mPtr(&t)
    {
    }

    template <typename U>
        requires(!Concepts::InstanceOf<U, CallerHierarchy>)
    operator const U &() const
    {
        const CallerHierarchyBase *ptr = mPtr;
        while (ptr) {
            if (ptr->mType == typeid(U))
                return static_cast<const CallerHierarchy<U> *>(ptr)->mData;
            ptr = ptr->mParent;
        }
        throw 0; // LOG_ERROR("Cannot find type '" << typeid(U).name() << "' in caller hierarchy. Containing:");
    }

    template <typename U>
    const CallerHierarchy<std::remove_const_t<std::remove_reference_t<U>>> append(U &&u) const
    {
        return {
            std::forward<U>(u), mPtr
        };
    }
};

template <typename T>
struct CallerHierarchyPtr : CallerHierarchyBasePtr {
    CallerHierarchyPtr(const CallerHierarchy<T> &t)
        : CallerHierarchyBasePtr { t }
    {
    }

    operator const T &() const
    {
        return static_cast<const CallerHierarchy<T> *>(mPtr)->mData;
    }

    operator const CallerHierarchy<T> &() const
    {
        return *static_cast<const CallerHierarchy<T> *>(mPtr);
    }

    template <typename U>
        requires std::convertible_to<T, U>
    operator CallerHierarchy<U>() const
    {
        return *static_cast<const CallerHierarchy<T> *>(mPtr);
    }
};

template <>
struct CallerHierarchyPtr<CallerHierarchyBasePtr> : CallerHierarchyBasePtr {
    CallerHierarchyPtr(const CallerHierarchyBasePtr &ptr)
        : CallerHierarchyBasePtr { ptr }
    {
    }
};

template <typename T>
CallerHierarchyPtr(const T &) -> CallerHierarchyPtr<T>;

template <typename T>
CallerHierarchyPtr(const CallerHierarchy<T> &) -> CallerHierarchyPtr<T>;

}