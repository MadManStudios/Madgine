#pragma once


namespace Engine {
namespace Plugins {

    struct NameCompare {
        template <typename T>
        bool operator()(const T &t1, const T &t2) const
        {
            return t1.name() < t2.name();
        }

        template <typename T>
        bool operator()(const T &t, std::string_view s) const
        {
            return t.name() < s;
        }

        template <typename T>
        bool operator()(std::string_view s, const T &t) const
        {
            return s < t.name();
        }

        using is_transparent = void;
    };
}
}