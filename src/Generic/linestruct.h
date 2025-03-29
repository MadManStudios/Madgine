#pragma once

namespace Engine {

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

#define START_STRUCT_EX(Tag, ...) \
    template <>                   \
    struct LineStruct<Tag, __VA_ARGS__>
#define START_STRUCT(Tag, ...) START_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)
#define LINE_STRUCT_EX(Tag, ...)      \
    START_STRUCT_EX(Tag, __VA_ARGS__) \
        : PrevLineStruct<Tag, __VA_ARGS__>
#define LINE_STRUCT(Tag, ...) LINE_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)
#define BASE_STRUCT(Tag, ...) ::Engine::PrevLineStruct<Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__>
#define GET_STRUCT_EX(Tag, ...) \
    ::Engine::LineStruct<Tag, __VA_ARGS__>
#define GET_STRUCT(Tag, ...) \
    GET_STRUCT_EX(Tag __VA_OPT__(, ) __VA_ARGS__, __LINE__)