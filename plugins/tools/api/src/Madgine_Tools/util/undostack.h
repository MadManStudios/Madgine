#pragma once

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT UndoStack {

        bool isDirty() const;

        void addOperation();
        void onSave();

    private:
        bool mIsDirty = false;
    };

}
}