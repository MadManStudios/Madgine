#pragma once

namespace Engine {
namespace __generic_impl__ {

    template <typename Tag, size_t... Ids>
    struct LineStruct;

    template <typename Tag, size_t Line, size_t... Ids>
    using PrevLineStruct = LineStruct<Tag, Line - 1, Ids...>;

    template <typename Tag, size_t Line, size_t... Ids>
    struct LineStruct<Tag, Line, Ids...> : PrevLineStruct<Tag, Line, Ids...> {
    };

    template <typename Tag, size_t... Ids>
    struct LineStruct<Tag, 0, Ids...>;

}
}

#define START_STRUCT_EX(Tag, ...) \
    template <>                   \
    struct ::Engine::__generic_impl__::LineStruct<Tag, __VA_ARGS__>
#define START_STRUCT(Tag, ...) START_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)
#define LINE_STRUCT_EX(Tag, ...)      \
    START_STRUCT_EX(Tag, __VA_ARGS__) \
        : ::Engine::__generic_impl__::PrevLineStruct<Tag, __VA_ARGS__>
#define LINE_STRUCT(Tag, ...) LINE_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)
#define BASE_STRUCT(Tag, ...) ::Engine::__generic_impl__::PrevLineStruct<Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__>
#define GET_STRUCT_EX(Tag, ...) \
    ::Engine::__generic_impl__::LineStruct<Tag, __VA_ARGS__>
#define GET_STRUCT(Tag, ...) \
    GET_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)