#pragma once

#include <zep.h>

#include "Platform/filesystem/path.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TEXTEDITOR_EXPORT TextDocument : Zep::IZepComponent {

        TextDocument(Platform::Filesystem::Path path, TextEditor &editor);

        bool render();
        void renderContent(ImVec2 size = { -1.0f, -1.0f });

        void handleInputs();

        void goToLine(size_t lineNr);

        bool hasBreakpoint(size_t lineNr);

    protected:
        void Notify(std::shared_ptr<Zep::ZepMessage> message) override;

        Zep::ZepEditor &GetEditor() const override;

    private:
        Platform::Filesystem::Path mPath;

        Zep::ZepEditor_ImGui mEditor;

        Zep::ZepBuffer *mBuffer;
    };

}
}