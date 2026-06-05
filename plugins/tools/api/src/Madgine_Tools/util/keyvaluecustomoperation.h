#pragma once

#include "undostack.h"

namespace Engine {
namespace Tools {
    template <typename T>
    struct CustomOperation : UndoableOperation {

        template <typename U, typename R>
        CustomOperation(Closure<Reflect::Result(const Traced<T> &)> trace, U &&undo, R &&redo)
            : mTrace(std::move(trace))
            , mUndo([undo { std::forward<U>(undo) }](const Reflect::Value &v) { return Reflect::invoke(undo, v); })
            , mRedo([redo { std::forward<R>(redo) }](const Reflect::Value &v) { return Reflect::invoke(redo, v); })
        {
        }

        Reflect::Result undo() override;
        Reflect::Result redo() override;

    private:
        Closure<Reflect::Result(const Traced<T> &)> mTrace;
        std::function<Reflect::Result(const Reflect::Value &)> mUndo;
        std::function<Reflect::Result(const Reflect::Value &)> mRedo;
    };
}
}