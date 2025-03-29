#pragma once

namespace Engine {

template <typename T>
T *toPointer(T &t)
{
    return &t;
}

constexpr auto projectionToPointer = LIFT(toPointer);

template <typename T, typename D>
T *uniquePtrToPtr(const std::unique_ptr<T, D> &p)
{
    return p.get();
}

constexpr auto projectionUniquePtrToPtr = LIFT(uniquePtrToPtr);

template <typename T>
decltype(auto) pairSecond(T &&t)
{
    return std::invoke(&std::remove_reference_t<T>::second, std::forward<T>(t));    
}

constexpr auto projectionPairSecond = LIFT(pairSecond);

template <typename T>
decltype(auto) addressOf(T &t)
{
    return &t;
}

constexpr auto projectionAddressOf = LIFT(addressOf);

template <typename T>
decltype(auto) toRawPtr(const T &t)
{
    return t.get();
}

constexpr auto projectionToRawPtr = LIFT(toRawPtr);

template <typename T>
decltype(auto) deref(T &&t)
{
    return *t;
}

constexpr auto projectionDeref = LIFT(deref);

}