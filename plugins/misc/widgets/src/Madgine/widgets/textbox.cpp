#include "../widgetslib.h"

#include "textbox.h"

#include "Platform/input/inputevents.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

#define STB_TEXTEDIT_KEYTYPE uint16_t

#define STB_TEXTEDIT_STRING Engine::Widgets::Textbox
#define STB_TEXTEDIT_STRINGLEN(obj) obj->mText.size()
#define STB_TEXTEDIT_LAYOUTROW(result, obj, n) obj->layoutRow(result, n)
#define STB_TEXTEDIT_GETWIDTH(obj, n, i) obj->calculateWidth(i, n)
#define STB_TEXTEDIT_KEYTOTEXT(k) static_cast<char>(k)
#define STB_TEXTEDIT_GETCHAR(obj, i) obj->mText.at(i)
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_DELETECHARS(obj, i, n) obj->mText.erase(i, n)
#define STB_TEXTEDIT_INSERTCHARS(obj, i, c, n) (obj->mText.insert(i, c, n), true)

#define STB_TEXTEDIT_K(key) (static_cast<uint16_t>(key) << 8)

#define STB_TEXTEDIT_K_SHIFT STB_TEXTEDIT_K(Engine::Platform::Input::Key::Shift)
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

NAMED_UNIQUECOMPONENT(Textbox, Engine::Widgets::Textbox);

METATABLE_BEGIN_BASE(Engine::Widgets::Textbox, Engine::Widgets::WidgetBase)
    MEMBER(mText)
    NAMED_MEMBER(TextData, mTextRenderData)
    NAMED_MEMBER(Image, mImageRenderData)
METATABLE_END(Engine::Widgets::Textbox)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Textbox, Engine::Widgets::WidgetBase)
    FIELD(mTextRenderData)
    FIELD(mImageRenderData)
SERIALIZETABLE_END(Engine::Widgets::Textbox)

namespace Engine {
namespace Widgets {

    Textbox::Textbox(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent, { .acceptsPointerEvents = true, .allowsDragging = true })
    {
        stb_textedit_initialize_state(&mState, true);
    }

    bool Textbox::editable() const
    {
        return false;
    }

    void Textbox::setEditable(bool b)
    {
    }

    void Textbox::render(WidgetsRenderData &renderData)
    {
        const Math::Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());
        if (entry) {

            Math::Vector2 pos = getAbsolutePosition();
            Math::Vector3 size = getAbsoluteSize();

            mImageRenderData.renderImage(renderData, pos, size, *entry);

            if (mTextRenderData.available()) {
                mTextRenderData.render(renderData, mText, pos, size, isFocused() && mTextRenderData.animationInterval(1200ms, 600ms) ? mState.cursor : -1);
                if (mState.select_start != mState.select_end) {
                    const Math::Atlas2::Entry *blankEntry = manager().lookUpImage("blank_white");
                    if (blankEntry) {
                        Math::Color4 color = { 0.0f, 0.0f, 0.8f, 0.8f };
                        mTextRenderData.renderSelection(renderData, mText, pos, size, *blankEntry, mState.select_start, mState.select_end, ColorRenderData { color }.frame(pos, size.xy()));
                    }
                }
            }
        }

        WidgetBase::render(renderData);
    }

    void Textbox::injectPointerClick(const PointerClickEvent &arg)
    {
        stb_textedit_click(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectPointerClick(arg);
    }

    void Textbox::injectDragBegin(const DragBeginEvent &arg)
    {
        stb_textedit_click(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectDragBegin(arg);
    }

    void Textbox::injectDragMove(const DragMoveEvent &arg)
    {
        stb_textedit_drag(this, &mState, arg.mWindowPosition.x, arg.mWindowPosition.y);
        WidgetBase::injectDragMove(arg);
    }

    bool Textbox::injectKeyPress(const Platform::Input::KeyPressEvent &arg)
    {
        if (std::isalnum(arg.mText)
            || arg.mScancode == Platform::Input::Key::Control
            || arg.mScancode == Platform::Input::Key::LeftArrow
            || arg.mScancode == Platform::Input::Key::RightArrow
            || arg.mScancode == Platform::Input::Key::UpArrow
            || arg.mScancode == Platform::Input::Key::DownArrow
            || arg.mScancode == Platform::Input::Key::Backspace
            || arg.mScancode == Platform::Input::Key::Delete
            || arg.mScancode == Platform::Input::Key::Space) {
            uint32_t val = (static_cast<uint16_t>(arg.mScancode) << 8) | arg.mText;
            stb_textedit_key(this, &mState, val);
        }
        return WidgetBase::injectKeyPress(arg);
    }

    void Textbox::layoutRow(StbTexteditRow *row, size_t i)
    {
        if (!mTextRenderData.available())
            return;

        assert(i == 0);
        Math::Vector2 pos = Math::Vector2::ZERO;
        Math::Vector3 size = getAbsoluteSize();
        Math::Rect2 bb = mTextRenderData.calculateBoundingBox(mText, pos, size);
        row->baseline_y_delta = bb.mTopLeft.y + bb.mSize.y;
        row->num_chars = mText.size();
        row->x0 = bb.mTopLeft.x;
        row->x1 = bb.mTopLeft.x + bb.mSize.x;
        row->ymax = bb.mTopLeft.y + bb.mSize.y;
        row->ymin = bb.mTopLeft.y;
    }

    float Textbox::calculateWidth(size_t i, size_t n)
    {

        return mTextRenderData.calculateWidth(mText.at(i), getAbsoluteSize().z);
    }
}
}