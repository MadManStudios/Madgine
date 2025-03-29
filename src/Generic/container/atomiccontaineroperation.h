#pragma once

namespace Engine {

DERIVE_TYPENAME(ResetOperation)

template <typename C, typename... Args>
decltype(auto) resetOperation(C &c, bool controlled, Args &&...args)
{
    if constexpr (has_typename_ResetOperation<C>)
        return typename C::ResetOperation { c, controlled, std::forward<Args>(args)... };
    else
        return c;
}

DERIVE_TYPENAME(InsertOperation)

template <typename C, typename... Args>
decltype(auto) insertOperation(C &c, Args &&...args)
{
    if constexpr (has_typename_InsertOperation<C>)
        return typename C::InsertOperation { c, std::forward<Args>(args)... };
    else
        return c;
}

DERIVE_TYPENAME(MultiInsertOperation)

template <typename C, typename... Args>
decltype(auto) multiInsertOperation(C &c, Args &&...args)
{
    if constexpr (has_typename_MultiInsertOperation<C>)
        return typename C::MultiInsertOperation { c, std::forward<Args>(args)... };
    else
        return c;
}

DERIVE_TYPENAME(RemoveOperation)

template <typename C, typename... Args>
decltype(auto) removeOperation(C &c, Args &&...args)
{
    if constexpr (has_typename_RemoveOperation<C>)
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

    auto begin() {
        return mContainer.begin();
    }

    auto begin() const {
        return std::as_const(mContainer).begin();
    }

    auto end() {
        return mContainer.end();
    }

    auto end() const {
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