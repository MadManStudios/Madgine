#pragma once

#include "closure.h"

namespace Engine {

struct Guard {
    Guard(Closure<void()> &&init, Closure<void()> &&finalize = {})
        : mFinalize(std::move(finalize))
    {
        if (init)
            init();
    }

    ~Guard()
    {
        if (mFinalize)
            mFinalize();
    }

private:
    Closure<void()> mFinalize;
};

}