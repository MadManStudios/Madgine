#pragma once

namespace Engine {
namespace Containers {

    template <typename C, typename... Args>
    decltype(auto) resetOperation(C &c, bool controlled, Args &&...args)
    {
        if constexpr (requires { typename C::ResetOperation; })
            return typename C::ResetOperation { c, controlled, std::forward<Args>(args)... };
        else
            return c;
    }

    template <typename C, typename... Args>
    decltype(auto) insertOperation(C &c, Args &&...args)
    {
        if constexpr (requires { typename C::InsertOperation; })
            return typename C::InsertOperation { c, std::forward<Args>(args)... };
        else
            return c;
    }

    template <typename C, typename... Args>
    decltype(auto) multiInsertOperation(C &c, Args &&...args)
    {
        if constexpr (requires { typename C::MultiInsertOperation; })
            return typename C::MultiInsertOperation { c, std::forward<Args>(args)... };
        else
            return c;
    }

    template <typename C, typename... Args>
    decltype(auto) removeOperation(C &c, Args &&...args)
    {
        if constexpr (requires { typename C::RemoveOperation; })
            return typename C::RemoveOperation { c, std::forward<Args>(args)... };
        else
            return c;
    }

    template <typename C>
    struct AtomicContainerOperationBase {

        typedef C container_type;

        AtomicContainerOperationBase(C &c)
            : mContainer(c)
        {
        }

        AtomicContainerOperationBase(const AtomicContainerOperationBase &) = delete;
        AtomicContainerOperationBase(AtomicContainerOperationBase &&) = delete;

        size_t size()
        {
            return mContainer.size();
        }

        auto begin()
        {
            return mContainer.begin();
        }

        auto begin() const
        {
            return std::as_const(mContainer).begin();
        }

        auto end()
        {
            return mContainer.end();
        }

        auto end() const
        {
            return std::as_const(mContainer).end();
        }

    protected:
        C &mContainer;
    };

    template <typename Op>
    struct AtomicContainerOperation : Op {
        using Op::Op;

        using Op::operator=;
    };

    template <typename Op>
    struct underlying_container<AtomicContainerOperation<Op>> {
        typedef typename Op::container_type type;
    };

}
}