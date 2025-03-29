#pragma once

namespace Engine {

template <size_t Size>
struct fixed_string {

    constexpr fixed_string(const char (&data)[Size + 1])
    {
        strcpy_s(mData, Size + 1, data);
    }

    template <size_t Size2>
    constexpr fixed_string<Size + Size2 - 1> operator+(const char (&data)[Size2]) const
    {
        char result[Size + Size2];
        strcpy_s(result, Size, mData);
        strcpy_s(result + Size, Size2, data);
        return result;
    }

    template <size_t Size2>
    constexpr fixed_string<Size + Size2> operator+(fixed_string<Size2> data) const
    {
        char result[Size + Size2 + 1];
        strcpy_s(result, Size, mData);
        strcpy_s(result + Size, Size2 + 1, data.c_str());
        return result;
    }

    constexpr operator std::string_view() const& {
        return { mData, Size };
    }

    constexpr const char* c_str() const {
        return mData;
    }

    char mData[Size + 1];
};

template <size_t Size>
fixed_string(const char (&data)[Size]) -> fixed_string<Size - 1>;


namespace detail {
    template <unsigned... digits>
    struct to_chars {
        static const char value[];
    };

    template <unsigned... digits>
    constexpr char to_chars<digits...>::value[] = { ('0' + digits)..., 0 };

    template <size_t rem, unsigned... digits>
    struct to_fixed_string_helper : to_fixed_string_helper<rem / 10, rem % 10, digits...> { };

    template <unsigned... digits>
    struct to_fixed_string_helper<0, digits...> : to_chars<digits...> { };
}

template <size_t I>
constexpr fixed_string to_fixed_string = detail::to_fixed_string_helper<I>::value;

}