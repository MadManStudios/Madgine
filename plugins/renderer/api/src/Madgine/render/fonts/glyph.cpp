#include "../../renderlib.h"

#include "glyph.h"

#include "Meta/serialize/hierarchy/serializetable.h"
#include "Meta/serialize/serializetable_impl.h"

SERIALIZETABLE_BEGIN(Engine::Render::Glyph)
FIELD(mSize)
FIELD(mSize2)
FIELD(mUV)
FIELD(mUV2)
FIELD(mBearing)
FIELD(mAdvance)
FIELD(mFlipped)
FIELD(mFlipped2)
SERIALIZETABLE_END(Engine::Render::Glyph)