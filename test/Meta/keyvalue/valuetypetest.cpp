#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/keyvalue/functiontable_impl.h"
#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/keyvalue/valuetype.h"

#include "Meta/keyvalue/lambda.h"

#include "Meta/keyvalue/keyvaluepair.h"

int testF(int i)
{
    return 3 + i;
}

FUNCTIONTABLE(testF, i)

struct ValueType_Foo {
    int operator()(int i) { return testF(i); }
};

METATABLE_BEGIN(ValueType_Foo)
CALL_OPERATOR(i)
METATABLE_END(ValueType_Foo)

struct ValueType_Bar {
    int operator()() { return 4; }
};

METATABLE_BEGIN(ValueType_Bar)
CALL_OPERATOR()
METATABLE_END(ValueType_Bar)

TEST(ValueType, Construct)
{
    using namespace Engine;
    using namespace std::literals::string_view_literals;

    ValueType_Foo foo;

    ValueType test;
    test = "abc"sv;
    test.clear();
    test = true;
    test = 3;
    test = (uint64_t)4;
    test = 5.0f;
    test = Vector2 { 6.1f, 6.2f };
    test = Vector3 { 7.1f, 7.2f, 7.3f };

    test = 5.0f;
    ASSERT_EQ(test, test);
    ASSERT_EQ(test, 5);
    ASSERT_EQ(test, ValueType { 5 });

    /* test = &function<&testF>();
    ValueType result;
    test.call(result, 1);
    ASSERT_EQ(result, 4);
    test = &testF;
    test.call(result, 2);
    ASSERT_EQ(result, 5);
    test = foo;
    test.call(result, 3);
    ASSERT_EQ(result, 6);
    test = ValueType_Foo {};
    test.call(result, 4);
    ASSERT_EQ(result, 7);
    int dummy = 5;
    test = lambda([=](int i) { return testF(i + dummy); });
    test.call(result, 5);
    ASSERT_EQ(result, 13);

    test = lambda(ValueType_Bar{});
    test.call(result);
    ASSERT_EQ(result, 4);
    test = ValueType_Bar {};
    test.call(result);
    ASSERT_EQ(result, 4);*/

    
    std::vector<int> v = { 1, 2, 3 };
    test = v;
    *std::next(test.as<KeyValueVirtualSequenceRange>().begin()) = ValueType { 4 };
}
