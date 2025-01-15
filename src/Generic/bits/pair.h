#pragma once

#include "freebits.h"
#include "types.h"
#include "field.h"

namespace Engine {

template <typename First, size_t SecondSize = 1, typename Second = MinimalHoldingUIntType_t<SecondSize>>
struct BitPair {

    static_assert(SecondSize <= FreeBitCount_v<First>);

    static constexpr size_t FreeBitCount = FreeBitCount_v<First> - SecondSize;

    using second_field = BitField<FreeBitCount, SecondSize, Second>;

    template <typename... FirstArgs, typename... SecondArgs>
    BitPair(std::piecewise_construct_t, std::tuple<FirstArgs...> firstArgs, std::tuple<SecondArgs...> secondArgs)
    {
        auto helper = []<size_t... Is, typename T>(T * target, auto &tuple, std::index_sequence<Is...>)
        {
            new (target) T(std::get<Is>(std::move(tuple))...);
        };
        helper(&mFirst, firstArgs, std::index_sequence_for<FirstArgs...>());
        helper(&mFirst, secondArgs, std::index_sequence_for<SecondArgs...>());
    }

    BitPair()
        : BitPair(std::piecewise_construct, std::make_tuple(), std::make_tuple())
    {
    }

    ~BitPair()
    {
        mSecond.~second_field();
        mFirst.~First();
    }

    BitPair &operator=(BitPair &&other)
    {
        mFirst = std::move(other.mFirst);
        mSecond = std::move(other.mSecond);
    }

    union {
        First mFirst;
        second_field mSecond;
    };
};

}