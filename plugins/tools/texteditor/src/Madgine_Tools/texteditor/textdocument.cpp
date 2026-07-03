#include "../texteditorlib.h"

#include "textdocument.h"

#include "Platform/filesystem/fsapi.h"

#include "imgui/imgui.h"
#include "texteditor.h"

namespace Engine {
namespace Tools {

    TextDocument::TextDocument(Platform::Filesystem::Path path, TextEditor &editor)
        : mPath(std::move(path))
        , mEditor("C:\\Users\\Bub\\Desktop\\GitHub\\Madgine\\plugins\\tools\\texteditor\\data\\zep.cfg", sPixelScale())
    {
        auto &display = static_cast<Zep::ZepDisplay_ImGui &>(mEditor.GetDisplay());

        ImFont *font = editor.font();
        int fontPixelHeight = editor.fontPixelHeight();

        display.SetFont(Zep::ZepTextType::UI, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
        display.SetFont(Zep::ZepTextType::Text, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
        display.SetFont(Zep::ZepTextType::Heading1, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.75)));
        display.SetFont(Zep::ZepTextType::Heading2, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.5)));
        display.SetFont(Zep::ZepTextType::Heading3, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.25)));

        std::string content;

        if (Platform::Filesystem::exists(mPath)) {
            Stream stream = Platform::Filesystem::openFileRead(mPath);
            content = { stream.iterator(), stream.end() };
        }

        mBuffer = mEditor.InitWithText(mPath, content);

        mEditor.SetGlobalMode(Zep::ZepMode_Standard::StaticName());

        mEditor.GetConfig().autoHideCommandRegion = false;

        mEditor.RegisterCallback(this);
    }

    bool TextDocument::render()
    {
        bool open = true;

        if (ImGui::Begin(mPath.filename().c_str(), &open)) {
            renderContent(ImGui::GetContentRegionAvail());

            handleInputs();
        }
        ImGui::End();

        return open;
    }

    void TextDocument::renderContent(ImVec2 size)
    {

        if (ImGui::BeginChild("Debug Panel", { 20, 0 })) {
        }
        ImGui::EndChild();
        ImGui::SameLine();

        auto pos = ImGui::GetCursorScreenPos();

        mEditor.SetDisplayRegion(Zep::NVec2f { pos.x, pos.y }, Zep::NVec2f { pos.x + size.x - 20.0f, pos.y + size.y });

        mEditor.Display();

    }

    void TextDocument::handleInputs()
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy)) {
            mEditor.HandleInput();
        }
    }

    void TextDocument::goToLine(size_t line)
    {
        Zep::ByteRange range;
        if (mBuffer->GetLineOffsets(line - 1, range)) {
            mEditor.GetActiveWindow()->SetBufferCursor(Zep::GlyphIterator(mBuffer, range.first));
        }
    }

    bool TextDocument::hasBreakpoint(size_t lineNr)
    {
        bool found = false;
        const Zep::SpanInfo &line = mEditor.GetActiveWindow()->GetCursorLineInfo(lineNr - 1);
        mBuffer->ForEachMarker(Zep::RangeMarkerType::Mark, Zep::Direction::Forward, Zep::GlyphIterator(mBuffer, line.lineByteRange.first), Zep::GlyphIterator(mBuffer, line.lineByteRange.second), [&](const std::shared_ptr<Zep::RangeMarker> &marker) {
            if (marker->GetDescription() == "Breakpoint") {
                found = true;
                return false;
            }
            return true;
        });
        return found;
    }

    void TextDocument::Notify(std::shared_ptr<Zep::ZepMessage> message)
    {
        if (message->messageId == Zep::Msg::MouseDown) {
            if (mEditor.GetActiveWindow()->GetNumberRegion().rect.Contains(message->pos)) {

                float height = mEditor.GetActiveWindow()->GetCursorLineInfo(0).FullLineHeightPx();

                const Zep::SpanInfo &line = mEditor.GetActiveWindow()->GetCursorLineInfo((message->pos.y - mEditor.GetActiveWindow()->ToWindowY(0.0f)) / height);
                size_t lineNr = line.bufferLineNumber;

                std::shared_ptr<Zep::RangeMarker> breakpointMarker;

                mBuffer->ForEachMarker(Zep::RangeMarkerType::Mark, Zep::Direction::Forward, Zep::GlyphIterator(mBuffer, line.lineByteRange.first), Zep::GlyphIterator(mBuffer, line.lineByteRange.second), [&](const std::shared_ptr<Zep::RangeMarker> &marker) {
                    if (marker->GetDescription() == "Breakpoint") {
                        breakpointMarker = marker;
                        return false;
                    }
                    return true;
                });

                if (breakpointMarker) {
                    mBuffer->ClearRangeMarker(breakpointMarker);
                } else {
                    auto testMarker = std::make_shared<Zep::RangeMarker>(*mBuffer);
                    testMarker->markerType = Zep::RangeMarkerType::Mark;
                    testMarker->displayType = Zep::RangeMarkerDisplayType::Indicator;
                    testMarker->SetHighlightColor(Zep::ThemeColor::VisualSelectBackground);
                    testMarker->SetDescription("Breakpoint");
                    testMarker->SetRange(line.lineByteRange);
                }

                message->handled = true;
            }
        }
    }

    Zep::ZepEditor &TextDocument::GetEditor() const
    {
        return const_cast<TextDocument *>(this)->mEditor;
    }

}
}