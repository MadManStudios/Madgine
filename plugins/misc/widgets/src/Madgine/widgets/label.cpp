#include "../widgetslib.h"

#include "label.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Label, Engine::Widgets::WidgetBase)
MEMBER(mText)
NAMED_MEMBER(TextData, mTextRenderData)
METATABLE_END(Engine::Widgets::Label)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Label, Engine::Widgets::WidgetBase)
FIELD(mText)
FIELD(mTextRenderData)
SERIALIZETABLE_END(Engine::Widgets::Label)

namespace Engine {
namespace Widgets {

    std::string Label::getClass() const
    {
        return "Label";
    }

    void Label::render(WidgetsRenderData &renderData)
    {
        if (mTextRenderData.available()) {

            Vector2 pos = getAbsolutePosition();
            Vector3 size = getAbsoluteSize();

            mTextRenderData.render(renderData, mText, pos, size);
        }

        WidgetBase::render(renderData);
    }

}
}
