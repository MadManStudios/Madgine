#pragma once

#include "Madgine_Tools/util/undostack.h"
#include "Platform/filesystem/path.h"

typedef int ImGuiWindowFlags; // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

namespace Engine {
namespace Tools {

    struct MADGINE_RESOURCES_TOOLS_EXPORT ResourceFileBase {

        ResourceFileBase(ResourceEditor &editor, Platform::Filesystem::Path path);
                
        bool Begin(bool *open = nullptr, ImGuiWindowFlags flags = 0);
        void Focus();

        void save();
        virtual void saveAs(const Platform::Filesystem::Path &path) = 0;

        
        Dialog<> closeDialog();


        ResourceEditor &mEditor;
        Platform::Filesystem::Path mPath;
        UndoStack mHistory;
        bool mCloseRequested = false;
    };

    template <typename Editor>
    struct ResourceFile : ResourceFileBase {
        ResourceFile(Editor &editor, Platform::Filesystem::Path path)
            : ResourceFileBase(editor, std::move(path))
        {
        }

        Editor &editor()
        {
            return static_cast<Editor &>(mEditor);
        }
    };

}
}