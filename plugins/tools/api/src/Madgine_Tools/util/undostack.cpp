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

    void UndoStack::addContinuousOperation(std::unique_ptr<UndoableOperation> op)
    {
        assert(!mContinuousOperation);
        mContinuousOperation = std::move(op);
    }

    UndoableOperation *UndoStack::getContinuousOperation() const
    {
        return mContinuousOperation.get();
    }

    void UndoStack::commitContinuousOperation()
    {
        assert(mContinuousOperation);
        addOperation(std::move(mContinuousOperation));
        mContinuousOperation.reset();
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
        bool canUndo = mCurrentOperation != mOperations.begin();
        if (!canUndo)
            ImGui::BeginDisabled();
        ImGui::SetNextItemShortcut(ImGuiKey_Z | ImGuiMod_Ctrl);
        if (ImGui::Button(IMGUI_ICON_UNDO)) {
            undo();
        }
        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            ImGui::BeginTooltip();
            ImGui::Text("Stack Size: %zu", mOperations.size());
            ImGui::EndTooltip();
        }
        if (!canUndo)
            ImGui::EndDisabled();

        // ImGui::SameLine(); //Assuming in Layout
        bool canRedo = mCurrentOperation != mOperations.end();
        if (!canRedo)
            ImGui::BeginDisabled();
        ImGui::SetNextItemShortcut(ImGuiKey_Z | ImGuiMod_Ctrl | ImGuiMod_Shift);
        if (ImGui::Button(IMGUI_ICON_REDO)) {
            redo();
        }
        if (!canRedo)
            ImGui::EndDisabled();
    }

}
}