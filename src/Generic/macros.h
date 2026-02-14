#pragma once

#define STRINGIFY(x) #x
#define STRINGIFY2(a) STRINGIFY(a)

#define EVAL(...) __VA_ARGS__
#define EVAL2(...) EVAL(EVAL(__VA_ARGS__))

#define PARENS ()

#define APPLY(f, ...) f(__VA_ARGS__)

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define SINGLE_ARG(...) __VA_ARGS__

#define SELECT_0(a, ...) a
#define SELECT_1(a, b, ...) b
#define SELECT_2(a, b, c, ...) c
#define SELECT_3(a, b, c, d, ...) d
#define SELECT_4(a, b, c, d, e, ...) e
#define SELECT_5(a, b, c, d, e, f, ...) f
#define SELECT_6(a, b, c, d, e, f, g, ...) g
#define SELECT_7(a, b, c, d, e, f, g, h, ...) h
#define SELECT_8(a, b, c, d, e, f, g, h, i, ...) i
#define SELECT_9(a, b, c, d, e, f, g, h, i, j, ...) j
#define SELECT_10(a, b, c, d, e, f, g, h, i, j, k, ...) k

#define FIRST(...) SELECT_0(__VA_ARGS__)
#define HEAD(...) SELECT_0(__VA_ARGS__)
#define TAIL(a, ...) __VA_ARGS__

#define SELECT(n, ...) APPLY(CONCAT2(SELECT_, n), __VA_ARGS__)

#define FOR_EACH1(f, sep, a) f(a)
#define FOR_EACH2(f, sep, a, b) f(a) sep f(b)
#define FOR_EACH3(f, sep, a, b, c) \
    f(a) sep f(b)                  \
    sep f(c)
#define FOR_EACH4(f, sep, a, b, c, d) \
    f(a) sep f(b)                     \
    sep f(c)                          \
    sep f(d)

#define FOR_EACH(f, sep, ...) APPLY(CONCAT2(FOR_EACH, NUM_ARGS(__VA_ARGS__)), f, sep, __VA_ARGS__)

#define NUM_ARGS(...) SELECT_10(__VA_ARGS__ __VA_OPT__(, ) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define DEBUG_MACRO2(...) static_assert(false, #__VA_ARGS__);
#define DEBUG_MACRO(...) DEBUG_MACRO2(__VA_ARGS__)

#define CONSTANT_CASE(name, stream) \
    case name:                      \
        stream << #name;            \
        break;