#pragma once

#include "Madgine/nodegraph/pins.h"

struct ImRect;

namespace Engine {
namespace Tools {

    static const int sPinIconSize = 24;

    namespace ed = ax::NodeEditor;

    void ShowLabel(std::string_view label, ImColor color = { 0.0f, 0.0f, 0.0f });

    ImColor DataColor(uint32_t mask, ExtendedValueTypeDesc type = ExtendedValueTypeIndex { ExtendedValueTypeEnum::GenericType });
    ImColor FlowColor(uint32_t mask);

    ImVec2 DataPinIcon(ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    ImVec2 DataInstancePinIcon(ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    ImVec2 FlowPinIcon(uint32_t mask, bool connected);

    ImVec2 FlowOutPin(const char *name, uint32_t mask, bool connected);
    void FlowOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected);
    ImVec2 FlowInPin(const char *name, uint32_t mask, bool connected);
    void FlowInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected);
    ImVec2 DataOutPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    bool DataOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    ImVec2 DataInPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    bool DataInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    ImVec2 DataProviderPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    bool DataProviderPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    ImVec2 DataReceiverPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected);
    bool DataReceiverPin(const char *name, size_t nodeId, size_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected);

    void HoverPin(ExtendedValueTypeDesc type);

    ImRect BeginNodeEditor(ed::EditorContext *editor, const ImVec2 &size = ImVec2(0, 0));
    void EndNodeEditor(ImRect oldViewport);

    std::optional<ExtendedValueTypeDesc> BeginNode(const NodeGraph::NodeBase *node, uint32_t nodeId, std::optional<NodeGraph::PinDesc> dragPin = {}, std::optional<ExtendedValueTypeDesc> dragType = {});
    void EndNode();

    void NodeLinks(const NodeGraph::NodeBase *node, uint32_t nodeId);

    enum class IconType : ImU32 { Flow,
        Circle,
        Square,
        Grid,
        RoundSquare,
        Diamond };

    void Icon(const ImVec2 &size, IconType type, bool filled, const ImVec4 &color = ImVec4(1, 1, 1, 1), const ImVec4 &innerColor = ImVec4(0, 0, 0, 0));

}
}