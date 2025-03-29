#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/keyvalue/scopeptr.h"
#include "Meta/keyvalue/metatable_impl.h"

struct Foo {
    int i;
};

METATABLE_BEGIN(Foo)
MEMBER(i)
METATABLE_END(Foo)

struct Bar : Foo {
    float f;
};

METATABLE_BEGIN_BASE(Bar, Foo)
MEMBER(f)
METATABLE_END(Bar)

TEST(KeyValue, ScopePtr)
{
    Bar b;
    b.i = 1;
    b.f = 1.6f;
    Engine::ScopePtr ptr = &b;
    Foo *f = scope_cast<Foo>(ptr);
    ASSERT_EQ(f, &b);
}
