#include "../nodegraphlib.h"

#include "pins.h"

#include "Meta/serialize/serializetable_impl.h"

SERIALIZETABLE_BEGIN(Engine::NodeGraph::FlowOutPinPrototype)
FIELD(mTarget)
SERIALIZETABLE_END(Engine::NodeGraph::FlowOutPinPrototype)

SERIALIZETABLE_BEGIN(Engine::NodeGraph::DataInPinPrototype)
FIELD(mSource)
SERIALIZETABLE_END(Engine::NodeGraph::DataInPinPrototype)

SERIALIZETABLE_BEGIN(Engine::NodeGraph::DataOutPinPrototype)
FIELD(mTarget)
SERIALIZETABLE_END(Engine::NodeGraph::DataOutPinPrototype)

SERIALIZETABLE_BEGIN(Engine::NodeGraph::Pin)
FIELD(mNode)
FIELD(mIndex)
FIELD(mGroup)
SERIALIZETABLE_END(Engine::NodeGraph::Pin)