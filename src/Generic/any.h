#pragma once

#include "inheritable.h"

namespace Engine {

namespace __generic_impl__ {

    struct AnyHolderBase {
        virtual ~AnyHolderBase() = default;
    };

    template <typename T>
    struct AnyHolder : AnyHolderBase, Inheritable<T> {
        template <typename... Args>
        AnyHolder(Args &&...args)
            : Inheritable<T>(std::forward<Args>(args)...)
        {
        }
    };

}

/**
 * @brief can hold anything as value.
 * Any stores the value in a heap-allocated buffer. Keep in mind that this type, as everything in the Madgine, uses
 * the rules of forwarding. This means putting in a lvalue will generate an Any object holding a reference, while
 * putting in a rvalue will generate an actual instance by moving the input into the buffer.
 */
struct Any {
    template <typename T>
    struct inplace_t {
    };

    template <typename T>
    static constexpr inline inplace_t<T> inplace = {};

    /**
     * @brief creates an empty Any object holding nothing.
     */
    Any() = default;

    /**
     * @brief construct a new object of type T inplace and stores it in the Any object
     * @tparam T the type of the value to be held
     * @tparam Args the types of the arguments
     * @param _ unused, necessary to allow template parameter deduction for type T
     * @param args arguments to be passed into the constructor of T
     */
    template <typename T, typename... Args>
        requires(!std::same_as<std::decay_t<T>, Any>)
    Any(inplace_t<T>, Args &&...args)
        : mData(std::make_unique<__generic_impl__::AnyHolder<T>>(std::forward<Args>(args)...))
    {
    }

    /**
     * @brief construct a new object of type T by copying/moving data.
     * @tparam T the type of the value to be held
     * @param data the value that is stored in the buffer using forwarding.
     */
    template <typename T>
        requires(!std::same_as<std::decay_t<T>, Any>)
    Any(T &&data)
        : mData(std::make_unique<__generic_impl__::AnyHolder<T>>(std::forward<T>(data)))
    {
    }

    /**
     * @brief moves the buffer stored in other into the newly constructed Any object.
     * @param other the Any object to be moved from
     */
    Any(Any &&other) = default;

    /**
     * @brief moves the buffer stored in other into the Any object. The previously held buffer is deleted.
     * @param other the Any object to be moved from
     * @return
     */
    Any &operator=(Any &&other) = default;

    /**
     * @brief returns true, if an object is held.
     */
    explicit operator bool() const
    {
        return mData.operator bool();
    }

    /**
     * @brief retrieves a reference to the held object.
     * This fails if the provided type T does not match the held object type exactly.
     * @tparam T the type to be accessed
     * @return a reference to the held object.
     */
    template <typename T>
    T &as() const
    {
        __generic_impl__::AnyHolder<T> *holder = dynamic_cast<__generic_impl__::AnyHolder<T> *>(mData.get());
        if (!holder)
            std::terminate();
        return *holder;
    }

private:
    std::unique_ptr<__generic_impl__::AnyHolderBase> mData;
};
}