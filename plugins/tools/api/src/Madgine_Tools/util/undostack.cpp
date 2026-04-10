#include "../toolslib.h"

#include "undostack.h"

#include "../imguiicons.h"
#include "imgui/imgui.h"

namespace Engine {
namespace Tools {

    bool UndoStack::isDirty() const
    {
        return !mBasePointer || *mBasePointer != mCurrentOperation;
    }

    void UndoStack::addOperation(std::unique_ptr<UndoableOperation> op)
    {
        if (mBasePointer && std::distance(*mBasePointer, mOperations.begin()) > std::distance(mCurrentOperation, mOperations.begin()))
            mBasePointer.reset();
        mOperations.erase(mCurrentOperation, mOperations.end());
        mOperations.emplace_back(std::move(op));
        mCurrentOperation = mOperations.end();
        if (mOperations.size() == 1 && mBasePointer) {
            *mBasePointer = mOperations.begin();
        }
    }

    void UndoStack::onSave()
    {
        mBasePointer = mCurrentOperation;
    }

    void UndoStack::undo()
    {
        if (mCurrentOperation == mOperations.begin()) {
            return;
        }
        --mCurrentOperation;
        (*mCurrentOperation)->undo();
    }

    void UndoStack::redo()
    {
        if (mCurrentOperation == mOperations.end()) {
            return;
        }
        (*mCurrentOperation)->redo();
        ++mCurrentOperation;
    }

    void UndoStack::renderControls()
    {
        if (ImGui::Button(IMGUI_ICON_UNDO)) {
            undo();
        }
        //ImGui::SameLine(); //Assuming in Layout
        if (ImGui::Button(IMGUI_ICON_REDO)) {
            redo();
        }
    }

    void UndoStack::handleShortcuts()
    {
    }

}
}