#pragma once

namespace Engine {
namespace Tools {

    struct UndoableOperation {
        virtual ~UndoableOperation() = default;
        virtual void undo() = 0;
        virtual void redo() = 0;
    };

    struct MADGINE_TOOLS_EXPORT UndoStack {

        UndoStack() = default;
        UndoStack(const UndoStack &) = delete;

        UndoStack &operator=(const UndoStack &) = delete;

        

        bool isDirty() const;

        void addOperation(std::unique_ptr<UndoableOperation> op);
        void onSave();
        
        void undo();
        void redo();

        void renderControls();
        void handleShortcuts();

    private:
        std::list<std::unique_ptr<UndoableOperation>> mOperations;
        std::list<std::unique_ptr<UndoableOperation>>::iterator mCurrentOperation = mOperations.begin();
        std::optional<std::list<std::unique_ptr<UndoableOperation>>::iterator> mBasePointer { std::in_place, mOperations.begin() };        
    };

}
}