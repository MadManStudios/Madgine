#pragma once

namespace Engine {
namespace Tools {

    const Behavior::NodeGraph::NodeDebugLocation *visualizeDebugLocation(DebuggerView &view, const Debug::ContextInfo &context, const Behavior::NodeGraph::NodeDebugLocation &location, const Debug::DebugLocation *inlineLocation);

}
}