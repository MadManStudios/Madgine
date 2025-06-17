#pragma once

#if defined(LibA_EXPORTS)
#    define LIBA_EXPORT DLL_EXPORT
#else
#    define LIBA_EXPORT DLL_IMPORT
#endif

#include "uniquecomponentshared.h"

struct LIBA_EXPORT LibAComponent : Test::TestComponent<LibAComponent> {

    LibAComponent(TestDriver &driver)
    {
    }
};