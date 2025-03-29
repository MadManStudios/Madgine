#include "../texteditorlib.h"

#include "texteditor.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"

UNIQUECOMPONENT(Engine::Tools::TextEditor)


METATABLE_BEGIN_BASE(Engine::Tools::TextEditor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::TextEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::TextEditor, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::TextEditor)


namespace Engine {
namespace Tools {

    float sFontSize = 16.0f;

    float dpi_pixel_height_from_point_size(float pointSize, float pixelScaleY)
    {
        const auto fontDotsPerInch = 72.0f;
        auto inches = pointSize / fontDotsPerInch;
        return inches * (pixelScaleY * 96.0f);
    }

    Zep::NVec2f sPixelScale() {
        return Zep::NVec2f { 1.0f };
    }

    TextEditor::TextEditor(ImRoot &root)
        : Tool<TextEditor>(root)
    {
        
    }

    Threading::Task<bool> TextEditor::init()
    {
        if (!co_await ToolBase::init())
            co_return false;

        //TODO
        auto fontPath = "C:\\Users\\Bub\\Desktop\\GitHub\\Madgine\\plugins\\tools\\texteditor\\data\\CascadiaMono.ttf";

        mFontPixelHeight = (int)dpi_pixel_height_from_point_size(sFontSize, sPixelScale().y);

        auto &io = ImGui::GetIO();
        static ImVector<ImWchar> ranges;
        ranges.clear();
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault()); // Add one of the default ranges
        builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic()); // Add one of the default ranges
        builder.AddRanges(Zep::greek_range);
        builder.BuildRanges(&ranges); // Build the final result (ordered ranges with all the unique characters submitted)

        ImFontConfig cfg;
        cfg.OversampleH = 4;
        cfg.OversampleV = 4;

        mFont = io.Fonts->AddFontFromFileTTF(fontPath, float(mFontPixelHeight), &cfg, ranges.Data);
        
        co_return true;
    }

    void TextEditor::render()
    {
        ImGui::PushFont(mFont);
        std::erase_if(mDocuments, [](auto &doc) { return !doc.second.render(); });        
        ImGui::PopFont();
    }

    std::string_view TextEditor::key() const
    {
        return "TextEditor";
    }

    TextDocument &TextEditor::openDocument(const Filesystem::Path &path)
    {
        return mDocuments.try_emplace(path, path, this).first->second;
    }

    TextDocument *TextEditor::getDocument(const Filesystem::Path &path)
    {
        auto it = mDocuments.find(path);
        return it == mDocuments.end() ? nullptr : &it->second;
    }

    ImFont *TextEditor::font() const
    {
        return mFont;
    }

    int TextEditor::fontPixelHeight() const
    {
        return mFontPixelHeight;
    }

}
}
