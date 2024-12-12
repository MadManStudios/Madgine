#pragma once

#include "coroutines/generator.h"

namespace Engine {
namespace StringUtil {

    constexpr bool isspace(char c)
    {
        return c == ' ' || c == '\n' || c == '\t';
    }

    constexpr bool startsWith(std::string_view s, std::string_view prefix)
    {
        return s.substr(0, prefix.size()) == prefix;
    }

    constexpr bool endsWith(std::string_view s, std::string_view suffix)
    {
        return s.size() >= suffix.size() ? (s.substr(s.size() - suffix.size(), suffix.size()) == suffix) : false;
    }

    constexpr bool contains(std::string_view s, std::string_view content)
    {
        return s.find(content) != std::string_view::npos;
    }

    CONSTEXPR_ALGORITHM std::string_view trim(std::string_view s)
    {
        auto begin = std::ranges::find_if_not(s, isspace);
        auto end = std::find_if_not(s.rbegin(), s.rend(), isspace);
        return { &*begin, static_cast<size_t>(end.base() - begin) };
    }

    constexpr std::string_view substr(std::string_view s, int start, int end)
    {
        if (end < 0)
            end += s.size();
        return s.substr(start, end - start);
    }

    inline std::string &replace(std::string &s, char old, char rep)
    {
        for (char &c : s) {
            if (c == old)
                c = rep;
        }
        return s;
    }

    template <typename String>
    inline String &replace(String &s, std::string_view old, std::string_view rep)
    {
        size_t pos = s.find(old);
        size_t oldLen = old.size();
        size_t repLen = rep.size();

        while (pos != std::string::npos) {
            s.replace(pos, oldLen, rep);
            pos = s.find(old, pos + repLen);
        }
        return s;
    }

    inline std::string replace(std::string &&s, char old, char rep)
    {
        replace(s, old, rep);
        return std::move(s);
    }

    inline std::string replace(std::string &&s, std::string_view old, std::string_view rep)
    {
        replace(s, old, rep);
        return std::move(s);
    }

    inline std::string replace(const std::string &s, char old, char rep)
    {
        return replace(std::string { s }, old, rep);
    }

    inline std::string replace(const std::string &s, std::string_view old, std::string_view rep)
    {
        return replace(std::string { s }, old, rep);
    }

    inline std::string toLower(std::string &&s)
    {
        std::ranges::transform(s, s.begin(), ::tolower);
        return std::move(s);
    }

    inline std::string toLower(std::string_view s)
    {
        return toLower(std::string { s });
    }

    inline std::string toUpper(std::string &&s)
    {
        std::ranges::transform(s, s.begin(), ::toupper);
        return std::move(s);
    }

    template <typename Stream>
    struct StreamJoiner {

        StreamJoiner(Stream &out, const char *separator)
            : mOut(out)
            , mSeparator(separator)
        {
        }

        Stream &next()
        { 
            if (mFirst) {
                mFirst = false;
            } else {
                mOut << mSeparator;
            }
            return mOut;
        }

        bool empty() const {
            return mFirst;
        }

        Stream &mOut;
        const char *mSeparator;
        bool mFirst = true;
    };

    template <typename Cont>
    std::string join(Cont &&cont, const char *sep)
    {
        std::ostringstream ss;
        StreamJoiner out { ss, sep };
        for (auto &&element : cont) {
            out.next() << element;
        }
        return ss.str();
    }

    template <size_t S>
    constexpr std::array<std::string_view, S> tokenize(std::string_view string, char token)
    {
        std::array<std::string_view, S> result = {};
        size_t pivot = 0;
        for (size_t i = 0; i < S; ++i) {
            while (isspace(string[pivot]))
                ++pivot;
            size_t newPivot = string.find(token, pivot);
            if ((i == S - 1) != (newPivot == std::string_view::npos))
                throw "Not enough elements for tokenize";
            if (newPivot == std::string_view::npos)
                newPivot = string.size();
            size_t actualEnd = newPivot;
            while (actualEnd > pivot && isspace(string[actualEnd - 1]))
                --actualEnd;
            result[i] = string.substr(pivot, actualEnd - pivot);
            pivot = newPivot + 1;
        }
        return result;
    }

    inline Generator<std::string_view> tokenize(std::string_view string, char token)
    {
        size_t pivot = 0;
        while (pivot < string.size()) {
            while (isspace(string[pivot]))
                ++pivot;
            size_t newPivot = string.find(token, pivot);
            if (newPivot == std::string_view::npos)
                newPivot = string.size();
            size_t actualEnd = newPivot;
            while (actualEnd > pivot && isspace(string[actualEnd - 1]))
                --actualEnd;
            co_yield string.substr(pivot, actualEnd - pivot);
            pivot = newPivot + 1;
        }
    }

}

constexpr size_t strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0')
        ++len;
    return len;
}

constexpr const char *strrchr(const char *s, char c)
{
    size_t len = strlen(s);
    const char *p = s + len;
    while (p != s) {
        --p;
        if (*p == c)
            return p;
    }
    return nullptr;
}

constexpr bool streq(const char *lhs, const char *rhs)
{
    int i = -1;
    do {
        ++i;
        if (lhs[i] != rhs[i])
            return false;
    } while (lhs[i]);
    return true;
}

constexpr bool strcpy_s(char *dest, size_t size, const char *source)
{
    for (size_t i = 0; i < size; ++i) {
        dest[i] = source[i];
    }
    return true;
}

constexpr std::strong_ordering strcmp(const char *first, const char *second)
{
    while (*first || *second) {
        std::strong_ordering comp = *first <=> *second;
        if (comp != 0)
            return comp;
        ++first;
        ++second;
    }
    return std::strong_ordering::equal;
}

}