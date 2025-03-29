#pragma once

#include "Interfaces/filesystem/path.h"

#include <zep.h>

namespace Engine {
namespace Tools {

    struct MADGINE_TEXTEDITOR_EXPORT TextDocument : Zep::IZepComponent {

        TextDocument(Filesystem::Path path, TextEditor *editor);

        bool render();

        void goToLine(size_t lineNr);

        bool hasBreakpoint(size_t lineNr);

    protected:
        void Notify(std::shared_ptr<Zep::ZepMessage> message) override;

        Zep::ZepEditor &GetEditor() const override;

    private:
        Filesystem::Path mPath;

        Zep::ZepEditor_ImGui mEditor;

        Zep::ZepBuffer *mBuffer;
    };

}
}