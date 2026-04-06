#include "../toolslib.h"

#include "undostack.h"

namespace Engine {
namespace Tools {

    bool UndoStack::isDirty() const
    {
        return mIsDirty;
    }

    void UndoStack::addOperation()
    {
        mIsDirty = true;
    }

    void UndoStack::onSave()
    {
        mIsDirty = false;
    }

}
}