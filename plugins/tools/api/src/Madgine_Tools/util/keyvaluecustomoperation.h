#pragma once

#include "undostack.h"

namespace Engine {
namespace Tools {
    template <typename T>
    struct KeyValueCustomOperation : UndoableOperation {

        template <typename U, typename R>
        KeyValueCustomOperation(Closure<KeyValueResult(const Traced<T> &)> trace, U &&undo, R &&redo)
            : mTrace(std::move(trace))
            , mUndo([undo { std::forward<U>(undo) }](const ValueType &v) { return ValueType_unwrap(undo, v); })
            , mRedo([redo { std::forward<R>(redo) }](const ValueType &v) { return ValueType_unwrap(redo, v); })
        {
        }

        KeyValueResult undo() override;
        KeyValueResult redo() override;

    private:
        Closure<KeyValueResult(const Traced<T> &)> mTrace;
        std::function<KeyValueResult(const ValueType &)> mUndo;
        std::function<KeyValueResult(const ValueType &)> mRedo;
    };
}
}