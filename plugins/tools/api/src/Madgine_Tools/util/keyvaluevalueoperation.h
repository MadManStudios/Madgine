#pragma once

#include "undostack.h"

namespace Engine {
namespace Tools {
    template <typename T>
        requires(!std::is_reference_v<T>)
    struct ReflectValueOperation : UndoableOperation {

        ReflectValueOperation(Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T &> &)>)> trace, T value)
            : mTrace(std::move(trace))
            , mValue(std::forward<T>(value))
        {
        }

        Reflect::Result undo() override
        {
            return mTrace([&](const Traced<T &> &t) {
                std::swap(t.get(), mValue);
                return std::make_pair(Reflect::Result {}, true);
            });
        }
        Reflect::Result redo() override
        {
            return mTrace([&](const Traced<T &> &t) {
                std::swap(t.get(), mValue);
                return std::make_pair(Reflect::Result {}, true);
            });
        }

    private:
        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T &> &)>)> mTrace;
        T mValue;
    };
}
}