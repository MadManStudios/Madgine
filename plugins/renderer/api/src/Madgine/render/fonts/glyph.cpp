#include "../../renderlib.h"

#include "glyph.h"

#include "Meta/serialize/hierarchy/serializetable.h"

#include "Meta/serialize/serializetable_impl.h"

SERIALIZETABLE_BEGIN(Engine::Render::Glyph)
    FIELD(mSize)
    FIELD(mUV)    
    FIELD(mBearing)    
    FIELD(mAdvance)    
    FIELD(mFlipped)    
SERIALIZETABLE_END(Engine::Render::Glyph)