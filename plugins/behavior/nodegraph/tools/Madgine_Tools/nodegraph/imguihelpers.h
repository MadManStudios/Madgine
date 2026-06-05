#pragma once

#include "Madgine/nodegraph/pins.h"

struct ImRect;

namespace Engine {
namespace Tools {

    static const int sPinIconSize = 24;

    namespace ed = ax::NodeEditor;

    void ShowLabel(std::string_view label, ImColor color = { 0.0f, 0.0f, 0.0f });

    ImColor DataColor(uint32_t mask, Reflect::ExtendedType type = Reflect::ExtendedTypeIndex { Reflect::ExtendedTypeEnum::GenericType });
    ImColor FlowColor(uint32_t mask);

    void DataPinIcon(Reflect::ExtendedType type, uint32_t mask, bool connected);
    void FlowPinIcon(uint32_t mask, bool connected);

    void FlowOutPin(const char *name, uint32_t mask, bool connected);
    void FlowOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected);
    void FlowInPin(const char *name, uint32_t mask, bool connected);
    void FlowInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected);
    void DataOutPin(const char *name, Reflect::ExtendedType type, uint32_t mask, bool connected);
    bool DataOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, Reflect::ExtendedType type, uint32_t mask, bool connected);
    void DataInPin(const char *name, Reflect::ExtendedType type, uint32_t mask, bool connected);
    bool DataInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, Reflect::ExtendedType type, uint32_t mask, bool connected);

    void HoverPin(Reflect::ExtendedType type);

    std::optional<Reflect::ExtendedType> BeginNode(const Behavior::NodeGraph::NodeBase *node, uint32_t nodeId, std::optional<Behavior::NodeGraph::PinDesc> dragPin = {}, std::optional<Reflect::ExtendedType> dragType = {});
    void EndNode();

    void NodeLinks(const Behavior::NodeGraph::NodeBase *node, uint32_t nodeId);

    enum class IconType : ImU32 { Flow,
        Circle,
        Square,
        Grid,
        RoundSquare,
        Diamond };

    void Icon(const ImVec2 &size, IconType type, bool filled, const ImVec4 &color = ImVec4(1, 1, 1, 1), const ImVec4 &innerColor = ImVec4(0, 0, 0, 0));

}
}