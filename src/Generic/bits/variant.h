#pragma once

#include "freebits.h"
#include "pair.h"
#include "types.h"

namespace Engine {

template <typename... Ty>
struct BitVariant {

    static constexpr size_t NeededBitCount = std::bit_width(sizeof...(Ty) + 1 - 1);
    static constexpr size_t TotalBitCount = std::max({ sizeof(Ty) * 8 - FreeBitCount_v<Ty>... }) + NeededBitCount;

    using type = MinimalHoldingUIntType_t<TotalBitCount>;

    static_assert(((FreeBitCount_v<Ty> + (sizeof(type) - sizeof(Ty)) * 8 >= NeededBitCount) && ...));

    static constexpr size_t FreeBitCount = sizeof(type) * 8 - TotalBitCount;

    BitVariant()
    {
        set<0>();
    }

    ~BitVariant()
    {
        reset();
    }

    BitVariant &operator=(BitVariant &&other)
    {
        size_t i = other.mIndex;
        if (i != 0) {
            using F = void (*)(type *);
            F fs[] = {
                [this](BitVariant *v, type *other) {
                    Ty *p = address<Ty>(other->mData);
                    v->set<Ty>(std::move(*p));
                    p->~Ty();
                }...
            };
            fs[i - 1](this, &other->mData);
            other.mIndex = 0;
            mIndex = i;
        }
    }

    template <typename T>
    BitVariant &operator=(T &&other)
    {
        static_assert((std::is_constructible_v<Ty, T &&> + ...) == 1);
        static constexpr type index = ((std::is_constructible_v<Ty, T &&> * type_pack<Ty...>::template index<type, Ty>)+...);
        set<index>(std::forward<T>(other));
        return *this;
    }

    template <typename T>
    constexpr bool is() const
    {
        return type_pack<Ty...>::template index<type, T> == mIndex - 1;
    }

    template <typename T>
    constexpr const T &as() const
    {
        if (!is<T>())
            throw 0;
        return ref<T>();
    }

    template <size_t I, typename... Args>
    void emplace(Args &&...args)
    {
        set<I>(std::forward<Args>(args)...);
    }

    template <typename F>
    decltype(auto) visit(F &&f) const
    {
        using R = std::common_type_t<std::invoke_result_t<F &&, const Ty &>...>;
        using Fs = R (*)(const BitVariant *, F &&);
        Fs fs[] = {
            [](const BitVariant *v, F &&f) {
                return std::forward<F>(f)(v->ref<Ty>());
            }...
        };
        return fs[mIndex - 1](this, std::forward<F>(f));
    }

private:
    template <typename T>
    constexpr T &ref()
    {
        return *address<T>();
    }

    template <typename T>
    constexpr const T &ref() const
    {
        return *address<T>();
    }

    template <typename T>
    constexpr T *address()
    {
        return reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(&mData) + (sizeof(type) - sizeof(T)));
    }

    template <typename T>
    constexpr const T *address() const
    {
        return reinterpret_cast<const T *>(reinterpret_cast<const uint8_t *>(&mData) + (sizeof(type) - sizeof(T)));
    }

    template <size_t I, typename... Args>
    void set(Args &&...args)
    {
        reset();
        using T = typename type_pack<Ty...>::template select<I>;
        new (address<T>()) T { std::forward<Args>(args)... };
        mIndex = I + 1;
    }

    void reset()
    {        
        if (mIndex != 0) {
            using F = void (*)(BitVariant *);
            F fs[] = {
                [](BitVariant *v) {
                    v->address<Ty>()->~Ty();
                }...
            };
            fs[mIndex - 1](this);
            mIndex = 0;
        }
    }

private:
    union {
        type mData = 0;
        BitField<FreeBitCount, NeededBitCount> mIndex;
    };
};

}