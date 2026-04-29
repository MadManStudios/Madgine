#pragma once

#include "undostack.h"

namespace Engine {
namespace Tools {
    template <typename T>
        requires(!std::is_reference_v<T>)
    struct KeyValueValueOperation : UndoableOperation {

        KeyValueValueOperation(Closure<KeyValueResult(CallableView<std::pair<KeyValueResult, bool>(const Traced<T &> &)>)> trace, T value)
            : mTrace(std::move(trace))
            , mValue(std::forward<T>(value))
        {
        }

        KeyValueResult undo() override
        {
            return mTrace([&](const Traced<T &> &t) {
                std::swap(t.get(), mValue);
                return std::make_pair(KeyValueResult {}, true);
            });
        }
        KeyValueResult redo() override
        {
            return mTrace([&](const Traced<T &> &t) {
                std::swap(t.get(), mValue);
                return std::make_pair(KeyValueResult {}, true);
            });
        }

    private:
        Closure<KeyValueResult(CallableView<std::pair<KeyValueResult, bool>(const Traced<T &> &)>)> mTrace;
        T mValue;
    };
}
}