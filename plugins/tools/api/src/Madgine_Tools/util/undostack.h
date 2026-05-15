#pragma once

namespace Engine {
namespace Tools {

    struct UndoableOperation {
        virtual ~UndoableOperation() = default;
        virtual KeyValueResult undo() = 0;
        virtual KeyValueResult redo() = 0;
    };

    struct MADGINE_TOOLS_EXPORT UndoStack {

        UndoStack() = default;
        UndoStack(const UndoStack &) = delete;

        UndoStack &operator=(const UndoStack &) = delete;

        

        bool isDirty() const;

        void addOperation(std::unique_ptr<UndoableOperation> op);
        void onSave();

        void addContinuousOperation(std::unique_ptr<UndoableOperation> op);
        UndoableOperation *getContinuousOperation() const;
        void commitContinuousOperation();
        
        KeyValueResult undo();
        KeyValueResult redo();

        void renderControls();
        void handleShortcuts();

    private:
        std::list<std::unique_ptr<UndoableOperation>> mOperations;
        std::list<std::unique_ptr<UndoableOperation>>::iterator mCurrentOperation = mOperations.begin();
        std::optional<std::list<std::unique_ptr<UndoableOperation>>::iterator> mBasePointer { std::in_place, mOperations.begin() };        

        std::unique_ptr<UndoableOperation> mContinuousOperation;
    };

}
}