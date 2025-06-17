#pragma once

#if defined(LibB_EXPORTS)
#    define LIBB_EXPORT DLL_EXPORT
#else
#    define LIBB_EXPORT DLL_IMPORT
#endif

#include "uniquecomponentshared.h"

struct LIBB_EXPORT LibBComponent : Test::TestComponent<LibBComponent> {

    LibBComponent(TestDriver &driver)
    {
    }
};