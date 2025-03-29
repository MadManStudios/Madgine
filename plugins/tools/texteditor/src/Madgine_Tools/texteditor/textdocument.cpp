#include "../texteditorlib.h"

#include "textdocument.h"

#include "imgui/imgui.h"

#include "texteditor.h"

#include "Interfaces/filesystem/fsapi.h"

namespace Engine {
namespace Tools {

    TextDocument::TextDocument(Filesystem::Path path, TextEditor *editor)
        : mPath(std::move(path))
        , mEditor("C:\\Users\\Bub\\Desktop\\GitHub\\Madgine\\plugins\\tools\\texteditor\\data\\zep.cfg", sPixelScale())
    {
        auto &display = static_cast<Zep::ZepDisplay_ImGui &>(mEditor.GetDisplay());

        ImFont *font = editor->font();
        int fontPixelHeight = editor->fontPixelHeight();

        display.SetFont(Zep::ZepTextType::UI, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
        display.SetFont(Zep::ZepTextType::Text, std::make_shared<Zep::ZepFont_ImGui>(display, font, fontPixelHeight));
        display.SetFont(Zep::ZepTextType::Heading1, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.75)));
        display.SetFont(Zep::ZepTextType::Heading2, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.5)));
        display.SetFont(Zep::ZepTextType::Heading3, std::make_shared<Zep::ZepFont_ImGui>(display, font, int(fontPixelHeight * 1.25)));

        std::string content;

        if (Filesystem::exists(mPath)) {
            Stream stream = Filesystem::openFileRead(mPath);
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

            if (ImGui::BeginChild("Debug Panel", { 20, 0 })) {
            }
            ImGui::EndChild();
            ImGui::SameLine();
            if (ImGui::BeginChild("Content")) {

                auto min = ImGui::GetCursorScreenPos();
                auto max = ImGui::GetContentRegionAvail();
                max.x = std::max(1.0f, max.x);
                max.y = std::max(1.0f, max.y);

                // Fill the window
                max.x = min.x + max.x;
                max.y = min.y + max.y;
                mEditor.SetDisplayRegion(Zep::NVec2f { min.x, min.y }, Zep::NVec2f { max.x, max.y });

                mEditor.Display();

                if (ImGui::IsWindowFocused())
                    mEditor.HandleInput();
            }
            ImGui::EndChild();
        }
        ImGui::End();

        return open;
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