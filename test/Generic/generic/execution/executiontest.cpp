#include <gtest/gtest.h>

#include "Interfaces/interfaceslib.h"

/*#include "Generic/execution/algorithm.h"
#include "Generic/execution/promise.h"

#include "Generic/execution/state.h"
#include "Generic/genericresult.h"

TEST(Execution, Basic)
{
    using namespace Engine::Execution;

    std::vector<int> cont { 1, 2, 3, 4 };
    int v = 12;
    auto inner = [](int a, int &i) {
        return just(a, i)
            | then([](const auto &...v) { return (v + ...); })
            | let_value([&](int sum) {
                  return sequence(just(sum) | write_var<"test">(), just(sum) | assign(i));
              });
    };

    auto graph = [&](std::vector<int> &cont) {
        return read_var<"test", int &>()
            | let_value([&](int &a) { return for_each(cont, [&](int &i) { return inner(a, i); }); })
            | Variable<"test">(12);
    };

    detach(graph(cont));

    std::vector<int> expected { 13, 15, 18, 22 };
    ASSERT_EQ(cont, expected);
}*/
