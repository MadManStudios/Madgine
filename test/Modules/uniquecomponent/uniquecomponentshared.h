#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

#if defined(TestShared_EXPORTS)
#    define TEST_EXPORT DLL_EXPORT
#else
#    define TEST_EXPORT DLL_IMPORT
#endif

struct TestDriver {
};

struct TestBase {
    virtual ~TestBase() = default;
};

DECLARE_UNIQUE_COMPONENT(Test, Test, TestBase, Engine::UniqueComponent::Constructor<TestDriver &>)

REGISTER_TYPE(TestBase)