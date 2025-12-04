#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#if __cpp_lib_memory_resource >= 201603L
#    include <memory_resource>
#else
#    include <experimental/memory_resource>
namespace std {
namespace experimental {
    namespace pmr {
        template <typename T>
        using vector = std::vector<T, polymorphic_allocator<T>>;
        using string = std::basic_string<char, char_traits<char>, polymorphic_allocator<char>>;
        template <typename Key,
            typename T,
            typename Hash = hash<Key>,
            typename Pred = equal_to<Key>>
        using unordered_map = std::unordered_map<Key, T, Hash, Pred,
            polymorphic_allocator<pair<const Key, T>>>;
    }
}
namespace pmr = experimental::pmr;
}
#endif
