#include "../widgetslib.h"

#include "condition.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"


METATABLE_BEGIN(Engine::Widgets::Condition)
MEMBER(mName)
MEMBER(mFormula)
MEMBER(mOperator)
MEMBER(mReferenceValue)
METATABLE_END(Engine::Widgets::Condition)

SERIALIZETABLE_BEGIN(Engine::Widgets::Condition)
FIELD(mName)
FIELD(mFormula)
FIELD(mOperator)
FIELD(mReferenceValue)
SERIALIZETABLE_END(Engine::Widgets::Condition)



namespace Engine {
namespace Widgets {

}
}