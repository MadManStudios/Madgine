#pragma once

namespace Engine {

template <typename F>
struct DelayedConstruct {

    operator std::invoke_result_t<F>()
    {
        return mConstructor();
    }

    F mConstructor;
};

}