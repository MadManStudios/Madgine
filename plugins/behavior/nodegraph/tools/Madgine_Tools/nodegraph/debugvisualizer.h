#pragma once

namespace Engine {
namespace Tools {

	const NodeGraph::NodeDebugLocation *visualizeDebugLocation(DebuggerView &view, const Debug::ContextInfo &context, const NodeGraph::NodeDebugLocation &location, const Debug::DebugLocation *inlineLocation);

}
}