#pragma once

#include "closure.h"

namespace Engine {

template <typename T>
struct DelayedConstruct {

    operator T() {
        return mConstructor();
    }

    Closure<T()> mConstructor;
};

}