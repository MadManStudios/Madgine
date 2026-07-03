#pragma once

#include "Generic/bytebuffer.h"

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "textdocument.h"

namespace Engine {
namespace Tools {

    MADGINE_TEXTEDITOR_EXPORT Zep::NVec2f sPixelScale();

    struct MADGINE_TEXTEDITOR_EXPORT TextEditor : Tool<TextEditor> {

        TextEditor(ImRoot &root);

        virtual Threading::Task<bool> init() override;

        virtual void render() override;

        std::string_view key() const override;

        TextDocument &openDocument(const Platform::Filesystem::Path &path);
        TextDocument *getDocument(const Platform::Filesystem::Path &path);

        ImFont *font() const;
        int fontPixelHeight() const;

    private:
        std::map<Platform::Filesystem::Path, TextDocument> mDocuments;

        int mFontPixelHeight;
        ImFont *mFont;

        Memory::ByteBuffer mFontData;
    };

}
}