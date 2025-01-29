#include "../../widgetslib.h"

#include "colortintrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::ColorTintRenderData, Engine::Widgets::RenderData)
MEMBER(mNormalColor)
MEMBER(mHighlightedColor)
MEMBER(mPressedColor)
MEMBER(mSelectedColor)
MEMBER(mDisabledColor)
METATABLE_END(Engine::Widgets::ColorTintRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::ColorTintRenderData, Engine::Widgets::RenderData)
FIELD(mNormalColor)
FIELD(mHighlightedColor)
FIELD(mPressedColor)
FIELD(mSelectedColor)
FIELD(mDisabledColor)
SERIALIZETABLE_END(Engine::Widgets::ColorTintRenderData)
