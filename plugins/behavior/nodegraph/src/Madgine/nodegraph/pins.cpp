#include "../nodegraphlib.h"

#include "pins.h"

#include "Meta/serialize/serializetable_impl.h"

SERIALIZETABLE_BEGIN(Engine::Behavior::NodeGraph::FlowOutPinPrototype)
    FIELD(mTarget)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::FlowOutPinPrototype)

SERIALIZETABLE_BEGIN(Engine::Behavior::NodeGraph::DataInPinPrototype)
    FIELD(mSource)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::DataInPinPrototype)

SERIALIZETABLE_BEGIN(Engine::Behavior::NodeGraph::Pin)
    FIELD(mNode)
    FIELD(mIndex)
    FIELD(mGroup)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::Pin)