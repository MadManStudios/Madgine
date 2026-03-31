#include "../nodegraphtoolslib.h"

#include "debugvisualizer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Generic/projections.h"

#include "Madgine/nodegraph/nodeinterpreter.h"
#include "Madgine/nodegraph/pins.h"

#include "Madgine_Tools/debugger/debuggerview.h"
#include "NodeEditor/imgui_node_editor.h"
#include "NodeEditor/imgui_node_editor_internal.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "imguihelpers.h"

namespace Engine {
namespace Tools {

    std::vector<TypedPtr> visualizeDebugLocation(DebuggerView &view, const Debug::ContextInfo &context, const Behavior::NodeGraph::NodeDebugLocation *location, TypedPtr inlineLocation)
    {
        if (!location)
            return {};

        std::vector<TypedPtr> children;
        const Behavior::NodeGraph::NodeGraph &graph = *location->mInterpreter->graph();

        if (inlineLocation) {
            return { location };

        } else {
            if (!location->mEditorContext) {
                ed::Config config;

                config.UserPointer = location->mInterpreter;

                config.LoadSettings = [](char *data, void *userPointer) {
                    const std::string &layout = static_cast<Behavior::NodeGraph::NodeInterpreterStateBase *>(userPointer)->graph()->mLayoutData;
                    if (data) {
                        strcpy(data, layout.c_str());
                    }
                    return layout.size();
                };

                location->mEditorContext = { ed::CreateEditor(&config), &ed::DestroyEditor };
            }

            ImGui::PushID(location);

            ImRect oldViewport = BeginNodeEditor(location->mEditorContext.get(), { 0, 250 });

            ed::NodeId selectedNodes[256];
            auto selectedNodesCount = ed::GetSelectedNodes(selectedNodes, 256);
            assert(selectedNodesCount < 255);
            ed::ClearSelection();

            children = { location };
            while (true) {
                std::vector<TypedPtr> newChildren;
                for (auto it = children.begin(); it != children.end();) {
                    const Behavior::NodeGraph::NodeDebugLocation *childLocation = it->as<const Behavior::NodeGraph::NodeDebugLocation>();
                    if (!childLocation || childLocation->mInterpreter != location->mInterpreter) {
                        ++it;
                        continue;
                    }

                    it = children.erase(it);

                    if (childLocation->mNode) {
                        uint32_t nodeId = graph.nodeIndex(childLocation->mNode);
                        BeginNode(childLocation->mNode, nodeId);

                        if (childLocation->mChild) {
                            ImGui::BeginVertical("child");
                            std::ranges::move(view.visualizeDebugLocation(context, childLocation->mChild, location), std::back_inserter(newChildren));
                            ImGui::EndVertical();
                        }

                        EndNode();

                        ed::SelectNode(60000 * nodeId, true);
                    }
                }

                if (newChildren.empty())
                    break;

                std::ranges::move(newChildren, std::back_inserter(children));   
            }

            ed::NodeId ids[256];
            auto handledNodesCount = ed::GetOrderedNodeIds(ids, 256);
            assert(handledNodesCount < 255);
            std::vector<size_t> handledNodeIds { ids, ids + handledNodesCount };
            std::ranges::sort(handledNodeIds);
            uint32_t handledIndex = 0;

            uint32_t nodeId = 1;
            for (Behavior::NodeGraph::NodeBase *node : graph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {

                if (handledIndex < handledNodesCount && static_cast<size_t>(handledNodeIds[handledIndex]) == 60000 * nodeId) {
                    ++handledIndex;
                } else {
                    BeginNode(node, nodeId);

                    EndNode();
                }

                ++nodeId;
            }

            nodeId = 1;
            for (Behavior::NodeGraph::NodeBase *node : graph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {
                NodeLinks(node, nodeId);
                ++nodeId;
            }

            ed::NodeId newSelectedNodes[256];
            auto newSelectedNodesCount = ed::GetSelectedNodes(newSelectedNodes, 256);
            assert(newSelectedNodesCount < 255);

            bool recenter = newSelectedNodesCount != selectedNodesCount;

            for (uint32_t i = 0; i < newSelectedNodesCount; ++i) {
                if (!recenter && selectedNodes[i] != newSelectedNodes[i])
                    recenter = true;
                uint32_t nodeId = static_cast<size_t>(newSelectedNodes[i]) / 60000;
                const Behavior::NodeGraph::NodeBase *node = graph.node(nodeId);
                if (node && node->flowInGroupCount() > 0 && node->flowInCount(0) > 0) {
                    for (Behavior::NodeGraph::Pin pin : node->flowInSources(0, 0)) {
                        ed::Flow(60000 * pin.mNode + 6000 * pin.mGroup + 1001 + pin.mIndex);
                    }
                }
            }

            if (recenter && false)
                ed::NavigateToSelection(true);

            EndNodeEditor(oldViewport);

            ImGui::PopID();
        }

        for (const TypedPtr &child : children)        
            view.visualizeDebugLocation(context, child, {});

        return {};
    }

}
}