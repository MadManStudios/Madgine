#pragma once

namespace Engine {
namespace Tools {

    TypedPtr visualizeDebugLocation(DebuggerView &view, const Debug::ContextInfo &context, const Behavior::NodeGraph::NodeDebugLocation &location, TypedPtr inlineLocation);

}
}