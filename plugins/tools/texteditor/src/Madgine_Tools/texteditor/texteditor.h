#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "textdocument.h"

namespace Engine {
namespace Tools {

    Zep::NVec2f sPixelScale();

    struct MADGINE_TEXTEDITOR_EXPORT TextEditor : Tool<TextEditor> {

        TextEditor(ImRoot &root);

        virtual Threading::Task<bool> init() override;

        virtual void render() override;

        std::string_view key() const override;

        TextDocument &openDocument(const Filesystem::Path &path);
        TextDocument *getDocument(const Filesystem::Path &path);

        ImFont *font() const;
        int fontPixelHeight() const;

    private:
        std::map<Filesystem::Path, TextDocument> mDocuments;

        int mFontPixelHeight;
        ImFont *mFont;
    };

}
}

REGISTER_TYPE(Engine::Tools::TextEditor)