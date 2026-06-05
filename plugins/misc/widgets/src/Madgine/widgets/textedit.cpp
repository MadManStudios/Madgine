#include "../widgetslib.h"

#include "textedit.h"

#include "Platform/input/inputevents.h"
#include "Platform/window/windowapi.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

#define STB_TEXTEDIT_KEYTYPE uint32_t

#define STB_TEXTEDIT_STRING Engine::Widgets::TextEdit
#define STB_TEXTEDIT_STRINGLEN(obj) obj->size()
#define STB_TEXTEDIT_LAYOUTROW(result, obj, n) obj->layoutRow(result, n)
#define STB_TEXTEDIT_GETWIDTH(obj, n, i) obj->calculateWidth(i, n)
#define STB_TEXTEDIT_KEYTOTEXT(k) static_cast<char>(k)
#define STB_TEXTEDIT_GETCHAR(obj, i) obj->at(i)
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_DELETECHARS(obj, i, n) obj->erase(i, n)
#define STB_TEXTEDIT_INSERTCHARS(obj, i, c, n) obj->insert(i, { c, static_cast<size_t>(n) })

#define STB_TEXTEDIT_K(key) (static_cast<uint32_t>(key) << 8)

#define STB_TEXTEDIT_K_SHIFT 0x10000
#define STB_TEXTEDIT_K_LEFT STB_TEXTEDIT_K(Engine::Platform::Input::Key::LeftArrow)
#define STB_TEXTEDIT_K_RIGHT STB_TEXTEDIT_K(Engine::Platform::Input::Key::RightArrow)
#define STB_TEXTEDIT_K_UP STB_TEXTEDIT_K(Engine::Platform::Input::Key::UpArrow)
#define STB_TEXTEDIT_K_DOWN STB_TEXTEDIT_K(Engine::Platform::Input::Key::DownArrow)
#define STB_TEXTEDIT_K_PGUP STB_TEXTEDIT_K(Engine::Platform::Input::Key::PageUp)
#define STB_TEXTEDIT_K_PGDOWN STB_TEXTEDIT_K(Engine::Platform::Input::Key::PageDown)
#define STB_TEXTEDIT_K_LINESTART STB_TEXTEDIT_K(Engine::Platform::Input::Key::Home)
#define STB_TEXTEDIT_K_LINEEND STB_TEXTEDIT_K(Engine::Platform::Input::Key::End)
#define STB_TEXTEDIT_K_TEXTSTART 0x4000
#define STB_TEXTEDIT_K_TEXTEND 0x8000000
#define STB_TEXTEDIT_K_DELETE STB_TEXTEDIT_K(Engine::Platform::Input::Key::Delete)
#define STB_TEXTEDIT_K_BACKSPACE STB_TEXTEDIT_K(Engine::Platform::Input::Key::Backspace) | 8
#define STB_TEXTEDIT_K_UNDO 0x1000000
#define STB_TEXTEDIT_K_REDO 0x2000000

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

NAMED_UNIQUECOMPONENT(TextEdit, Engine::Widgets::TextEdit);

METATABLE_BEGIN_BASE(Engine::Widgets::TextEdit, Engine::Widgets::WidgetBase)
    PROPERTY(Text, text, setText)
    NAMED_MEMBER(TextData, mTextRenderData)
    NAMED_MEMBER(Image, mImageRenderData)
    PROPERTY(Border, border, setBorder)
    MEMBER(mVerticalScroll)
    PROPERTY(Editable, editable, setEditable)
METATABLE_END(Engine::Widgets::TextEdit)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TextEdit, Engine::Widgets::WidgetBase)
    ENCAPSULATED_FIELD(Text, text, setText)
    FIELD(mTextRenderData)
    FIELD(mImageRenderData)
    ENCAPSULATED_FIELD(Border, border, setBorder)
    ENCAPSULATED_FIELD(Editable, editable, setEditable)
SERIALIZETABLE_END(Engine::Widgets::TextEdit)

namespace Engine {
namespace Widgets {

    TextEdit::TextEdit(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent, { .acceptsPointerEvents = true, .allowsDragging = true })
    {
        stb_textedit_initialize_state(&mState, false);
    }

    void TextEdit::setText(std::string_view text)
    {
        mText = text;
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
        stb_textedit_clamp(this, &mState);
    }

    std::string_view TextEdit::text() const
    {
        return mText;
    }

    size_t TextEdit::size() const
    {
        return mText.size();
    }

    char TextEdit::at(size_t i) const
    {
        return mText.at(i);
    }

    void TextEdit::erase(size_t where, size_t n)
    {
        mText.erase(where, n);
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
    }

    bool TextEdit::insert(size_t where, std::string_view s)
    {
        mText.insert(where, s);
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
        return true;
    }

    bool TextEdit::editable() const
    {
        return mEditable;
    }

    void TextEdit::setEditable(bool b)
    {
        mEditable = b;
    }

    void TextEdit::render(WidgetsRenderData &renderData)
    {
        const Math::Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());
        if (entry) {

            Math::Vector2 pos = getAbsolutePosition();
            Math::Vector3 size = getAbsoluteSize();

            mImageRenderData.renderImage(renderData, pos, size, *entry);

            if (mTextRenderData.available()) {
                if (mTextRenderData.lines().empty() && !mText.empty())
                    mTextRenderData.updateText(mText, getAbsoluteTextSize());

                Math::Vector2 textPos = pos + mBorder;
                Math::Vector3 textSize = getAbsoluteTextSize();
                auto keep = renderData.pushClipRect(textPos, textSize.xy());
                Math::Vector2 scrolledPos { textPos.x, textPos.y - mVerticalScroll };

                mTextRenderData.render(renderData, scrolledPos, textSize, isFocused() && mTextRenderData.animationInterval(1200ms, 600ms) ? mState.cursor : -1);
                if (mState.select_start != mState.select_end) {
                    const Math::Atlas2::Entry *blankEntry = manager().lookUpImage("blank_white");
                    if (blankEntry) {
                        Math::Color4 color = { 0.0f, 0.0f, 0.8f, 0.8f };
                        mTextRenderData.renderSelection(renderData, scrolledPos, textSize, *blankEntry, mState.select_start, mState.select_end, ColorRenderData { color }.frame(textPos, textSize.xy()));
                    }
                }
            }
        }

        WidgetBase::render(renderData);
    }

    void TextEdit::sizeChanged(const Math::Vector3 &pixelSize)
    {
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
    }

    void TextEdit::onActivate(Serialize::CallbackTiming, bool b)
    {
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
    }

    void TextEdit::injectPointerClick(const PointerClickEvent &arg)
    {
        stb_textedit_click(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectPointerClick(arg);
    }

    void TextEdit::injectDragBegin(const DragBeginEvent &arg)
    {
        stb_textedit_click(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectDragBegin(arg);
    }

    void TextEdit::injectDragMove(const DragMoveEvent &arg)
    {
        stb_textedit_drag(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectDragMove(arg);
    }

    bool TextEdit::injectKeyPress(const Platform::Input::KeyPressEvent &arg)
    {
        if (mEditable) {
            if (std::isalnum(arg.mText)
                || arg.mScancode == Platform::Input::Key::LeftArrow
                || arg.mScancode == Platform::Input::Key::RightArrow
                || arg.mScancode == Platform::Input::Key::UpArrow
                || arg.mScancode == Platform::Input::Key::DownArrow
                || arg.mScancode == Platform::Input::Key::Backspace
                || arg.mScancode == Platform::Input::Key::Delete
                || arg.mScancode == Platform::Input::Key::Space
                || arg.mScancode == Platform::Input::Key::Return) {
                uint32_t val = (static_cast<uint32_t>(arg.mControlKeys.mAlt) << 18)
                    | (static_cast<uint32_t>(arg.mControlKeys.mCtrl) << 17)
                    | (static_cast<uint32_t>(arg.mControlKeys.mShift) << 16)
                    | (static_cast<uint32_t>(arg.mScancode) << 8)
                    | arg.mText;
                stb_textedit_key(this, &mState, val);
            } else if (arg.mControlKeys.mCtrl && arg.mScancode == Platform::Input::Key::V) {
                std::string s = Platform::Window::OSWindow::getClipboardString();
                stb_textedit_paste(this, &mState, s.c_str(), s.size());
                mTextRenderData.updateText(mText, getAbsoluteTextSize());
            } else if (arg.mControlKeys.mCtrl && arg.mScancode == Platform::Input::Key::C) {
                std::string_view s = mText;
                int start = mState.select_start;
                int end = mState.select_end;
                if (start != end) {
                    if (start > end)
                        std::swap(start, end);
                    s = s.substr(start, end);
                }
                Platform::Window::OSWindow::setClipboardString(s);
            }
        }
        return WidgetBase::injectKeyPress(arg);
    }

    bool TextEdit::injectAxisEvent(const Platform::Input::AxisEvent &arg)
    {
        if (arg.mAxisType == Platform::Input::AxisEvent::WHEEL) {
            float textHeight = mTextRenderData.calculateTotalHeight(getAbsoluteTextSize().z);
            float maxScroll = Math::max(textHeight - getAbsoluteTextSize().y, 0.0f);
            mVerticalScroll = Math::clamp(mVerticalScroll - 50.0f * arg.mAxis1, 0.0f, maxScroll);
        }
        return WidgetBase::injectAxisEvent(arg);
    }

    void TextEdit::layoutRow(StbTexteditRow *row, size_t i)
    {
        if (!mTextRenderData.available())
            return;

        Math::Vector2 pos = Math::Vector2::ZERO;
        Math::Vector3 size = getAbsoluteSize();
        const std::vector<TextRenderData::Line> &lines = mTextRenderData.lines();
        if (lines.empty())
            return;

        auto it = std::ranges::find_if(lines, [&](const TextRenderData::Line &line) { return line.mBegin <= mText.data() + i && mText.data() + i <= line.mEnd; });
        Math::Rect2 bb = mTextRenderData.calculateBoundingBox(*it, lines.size(), std::distance(lines.begin(), it), pos, size);
        row->baseline_y_delta = bb.mSize.y;
        row->num_chars = it->mEnd - it->mBegin;
        if (std::next(it) != lines.end())
            ++row->num_chars;
        row->x0 = bb.mTopLeft.x;
        row->x1 = bb.mTopLeft.x + bb.mSize.x;
        row->ymax = bb.mSize.y;
        row->ymin = 0;
        if (it == lines.begin()) {
            row->baseline_y_delta += bb.mTopLeft.y - mVerticalScroll;
            row->ymax += bb.mTopLeft.y - mVerticalScroll;
            row->ymin += bb.mTopLeft.y - mVerticalScroll;
        }
    }

    float TextEdit::calculateWidth(size_t i, size_t n)
    {
        return mTextRenderData.calculateWidth(mText.at(i), getAbsoluteSize().z);
    }

    Math::Vector3 TextEdit::getAbsoluteTextSize()
    {
        Math::Vector3 size = getAbsoluteSize();
        size -= Math::Vector3 { 2 * mBorder, 0.0f };
        return size;
    }

    void TextEdit::setBorder(Math::Vector2 border)
    {
        mBorder = border;
        mTextRenderData.updateText(mText, getAbsoluteTextSize());
    }

    Math::Vector2 TextEdit::border() const
    {
        return mBorder;
    }

}
}