#include "../nodegraphtoolslib.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "imguihelpers.h"

#include "Madgine/nodegraph/nodeexecution.h"

#include "NodeEditor/imgui_node_editor.h"

namespace Engine {
namespace Tools {

    void ShowLabel(std::string_view label, ImColor color)
    {
        ImVec2 labelPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();
        labelPos.y -= ImGui::GetTextLineHeight();
        auto size = ImGui::CalcTextSize(label.data(), label.data() + label.size());

        auto padding = ImGui::GetStyle().FramePadding;
        auto spacing = ImGui::GetStyle().ItemSpacing;

        labelPos += ImVec2(spacing.x, -spacing.y);

        auto rectMin = labelPos - padding;
        auto rectMax = labelPos + size + padding;

        auto drawList = ImGui::GetWindowDrawList();
        //drawList->ChannelsSetCurrent(3);
        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
        //drawList->ChannelsSetCurrent(4);
        drawList->AddText(labelPos, IM_COL32(255, 255, 255, 255), label.data(), label.data() + label.size());
    };

    ImColor DataColor(uint32_t mask, ExtendedValueTypeDesc type)
    {
        if (type.mType == ValueTypeEnum::SenderValue) {
            return FlowColor(mask);
        }
        switch (mask) {
        case NodeGraph::NodeExecutionMask::NONE:
            return { 50, 50, 50, 255 };
            break;
        case NodeGraph::NodeExecutionMask::CPU:
            return { 0, 255, 0, 255 };
            break;
        case NodeGraph::NodeExecutionMask::GPU:
            return { 160, 160, 255, 255 };
            break;
        case NodeGraph::NodeExecutionMask::ALL:
            return { 0, 255, 255, 255 };
            break;
        default:
            throw 0;
        }
    }

    ImColor FlowColor(uint32_t mask)
    {
        switch (mask) {
        case NodeGraph::NodeExecutionMask::NONE:
            return { 50, 50, 50, 255 };
            break;
        case NodeGraph::NodeExecutionMask::CPU:
            return { 255, 0, 0, 255 };
            break;
        case NodeGraph::NodeExecutionMask::GPU:
            return { 200, 200, 0, 255 };
            break;
        case NodeGraph::NodeExecutionMask::ALL:
            return { 255, 127, 0, 255 };
            break;
        default:
            throw 0;
        }
    }

    ImVec2 DataPinIcon(ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        IconType icon = IconType::Circle;
        if (type.mType == ValueTypeEnum::SenderValue)
            icon = IconType::Diamond;

        Icon(ImVec2(sPinIconSize, sPinIconSize), icon, connected, DataColor(mask, type), ImColor(32, 32, 32, 255));
        ImVec2 align { 0.5f, 0.5f };
        return ImGui::GetItemRectMin() + align * ImGui::GetItemRectSize();
    }

    ImVec2 DataInstancePinIcon(ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        IconType icon = IconType::Square;
        if (type.mType == ValueTypeEnum::SenderValue)
            icon = IconType::Diamond;

        Icon(ImVec2(sPinIconSize, sPinIconSize), icon, connected, DataColor(mask, type), ImColor(32, 32, 32, 255));
        ImVec2 align { 0.5f, 0.5f };
        return ImGui::GetItemRectMin() + align * ImGui::GetItemRectSize();
    }

    ImVec2 FlowPinIcon(uint32_t mask, bool connected)
    {
        Icon(ImVec2(sPinIconSize, sPinIconSize), IconType::Flow, connected, FlowColor(mask), ImColor(32, 32, 32, 255));
        ImVec2 align { 0.5f, 0.5f };
        return ImGui::GetItemRectMin() + align * ImGui::GetItemRectSize();
    }

    ImVec2 FlowOutPin(const char *name, uint32_t mask, bool connected)
    {
        ImGui::Spring();
        if (name) {
            ImGui::TextUnformatted(name);
            ImGui::Spring(0);
        }
        return FlowPinIcon(mask, connected);
    }

    void FlowOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::flowOutId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Output);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = FlowOutPin(name, mask, connected);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetBR() - ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
    }

    ImVec2 FlowInPin(const char *name, uint32_t mask, bool connected)
    {
        ImVec2 pos = FlowPinIcon(mask, connected);
        if (name) {
            ImGui::Spring(0);
            ImGui::TextUnformatted(name);
        }
        ImGui::Spring();
        return pos;
    }

    void FlowInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::flowInId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Input);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = FlowInPin(name, mask, connected);
        //ed::PinPivotRect(pos, pos);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetTL() + ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
    }

    ImVec2 DataOutPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        ImGui::Spring();
        if (name) {
            ImGui::TextUnformatted(name);
            ImGui::Spring(0);
        }
        return DataPinIcon(type, mask, connected);
    }

    bool DataOutPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::dataOutId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Output);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = DataOutPin(name, type, mask, connected);
        //ed::PinPivotRect(pos, pos);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetBR() - ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
        return ImGui::IsItemHovered();
    }

    ImVec2 DataInPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        ImVec2 pos = DataPinIcon(type, mask, connected);
        if (name) {
            ImGui::Spring(0);
            ImGui::TextUnformatted(name);
        }
        ImGui::Spring();
        return pos;
    }

    bool DataInPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::dataInId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Input);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = DataInPin(name, type, mask, connected);
        //ed::PinPivotRect(pos, pos);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetTL() + ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
        return ImGui::IsItemHovered();
    }

    ImVec2 DataProviderPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        ImGui::Spring();
        if (name) {
            ImGui::TextUnformatted(name);
            ImGui::Spring(0);
        }
        return DataInstancePinIcon(type, mask, connected);
    }

    bool DataProviderPin(const char *name, uint32_t nodeId, uint32_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::dataProviderId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Output);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = DataProviderPin(name, type, mask, connected);
        //ed::PinPivotRect(pos, pos);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetBR() - ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
        return ImGui::IsItemHovered();
    }

    ImVec2 DataReceiverPin(const char *name, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        ImVec2 pos = DataInstancePinIcon(type, mask, connected);
        if (name) {
            ImGui::Spring(0);
            ImGui::TextUnformatted(name);
        }
        ImGui::Spring();
        return pos;
    }

    bool DataReceiverPin(const char *name, size_t nodeId, size_t pinId, uint32_t group, ExtendedValueTypeDesc type, uint32_t mask, bool connected)
    {
        int id = 60000 * nodeId + NodeGraph::NodeBase::dataReceiverId(pinId, group);
        ed::BeginPin(id, ed::PinKind::Input);

        ImGui::BeginHorizontal(id, ImVec2 { 0, 0 }, 0.5f);

        ImVec2 pos = DataReceiverPin(name, type, mask, connected);
        //ed::PinPivotRect(pos, pos);

        ImGui::EndHorizontal();

        ImRect rect = { ImGui::GetItemRectMin(), ImGui::GetItemRectMax() };

        //ImGui::GetWindowDrawList()->AddRectFilled(rect.GetTL(), rect.GetBR(), ImColor(229, 229, 229, 200));

        ImVec2 pivot = rect.GetTL() + ImVec2 { 0.5f * sPinIconSize, 0.5f * rect.GetHeight() };

        ed::PinPivotRect(pivot, pivot);

        ed::EndPin();
        return ImGui::IsItemHovered();
    }

    void HoverPin(ExtendedValueTypeDesc type)
    {
        ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorScreenPos(ImGui::GetMousePos());
        if (type.mType.isRegular()) {
            ShowLabel(type.toString());
        } else {
            ShowLabel("*");
        }
        ImGui::SetCursorPos(cursor);
    }

    ImRect BeginNodeEditor(ed::EditorContext *editor, const ImVec2 &size)
    {
        //Probably set this in the implementation to the exact Canvas-View area
        ImVec2 oldViewportPos = ImGui::GetCurrentContext()->MouseViewport->Pos;
        ImVec2 oldViewportSize = ImGui::GetCurrentContext()->MouseViewport->Size;

        ImGui::GetCurrentContext()->MouseViewport->Pos = { -10000, -10000 };
        ImGui::GetCurrentContext()->MouseViewport->Size = { 20000, 20000 };

        ed::SetCurrentEditor(editor);

        ed::Begin("Node editor", size);

        return { oldViewportPos, oldViewportPos + oldViewportSize };
    }

    void EndNodeEditor(ImRect oldViewport)
    {
        ed::End();

        ed::SetCurrentEditor(nullptr);

        ImGui::GetCurrentContext()->MouseViewport->Pos = oldViewport.Min;
        ImGui::GetCurrentContext()->MouseViewport->Size = oldViewport.GetSize();
    }

    std::optional<ExtendedValueTypeDesc> BeginNode(const NodeGraph::NodeBase *node, uint32_t nodeId, std::optional<NodeGraph::PinDesc> dragPin, std::optional<ExtendedValueTypeDesc> dragType)
    {
        ed::BeginNode(60000 * nodeId);

        ImGui::PushID(node);

        std::optional<ExtendedValueTypeDesc> hoveredPin;

        ImGui::BeginVertical("node");

        ImGui::BeginHorizontal("header");

        ImGui::Text(node->name());

        ImGui::EndHorizontal();

        ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.y * 2.0f);

        ImGui::BeginHorizontal("content");
        ImGui::Spring(0, 0);

        ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);

        ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0.0f, 0.5f));
        ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));

        for (uint32_t group = 0; group < node->flowInGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->flowInCount(group); ++index) {
                FlowInPin(node->flowInName(index, group).data(), nodeId, index, group, node->flowInMask(index, group), !node->flowInSources(index, group).empty());
            }
        }

        for (uint32_t group = 0; group < node->dataInGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->dataInCount(group); ++index) {
                NodeGraph::Pin source = node->dataInSource(index, group);

                ExtendedValueTypeDesc type = node->dataInType(index, group);

                if (DataInPin(node->dataInName(index, group).data(), nodeId, index, group, type, node->dataInMask(index, group), static_cast<bool>(source)))
                    hoveredPin = type;
            }

            if (dragPin && dragPin->mType == NodeGraph::PinType::DataInstance && node->dataInVariadic(group) && dragPin->mDir == NodeGraph::PinDir::Out) {
                uint32_t index = node->dataInCount(group);
                if (DataInPin(node->dataInName(index).data(), nodeId, index, group, *dragType, node->dataInMask(index, group), {}))
                    hoveredPin = *dragType;
            }
        }

        for (uint32_t group = 0; group < node->dataReceiverGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->dataReceiverCount(group); ++index) {
                ExtendedValueTypeDesc type = node->dataReceiverType(index, group);

                if (DataReceiverPin(node->dataReceiverName(index, group).data(), nodeId, index, group, type, node->dataReceiverMask(index, group), !node->dataReceiverSources(index, group).empty()))
                    hoveredPin = type;
            }

            if (dragPin && dragPin->mType == NodeGraph::PinType::Data && node->dataReceiverVariadic(group) && dragPin->mDir == NodeGraph::PinDir::Out) {
                uint32_t index = node->dataReceiverCount(group);
                if (DataReceiverPin(node->dataReceiverName(index, group).data(), nodeId, index, group, *dragType, node->dataReceiverMask(index, group), false))
                    hoveredPin = *dragType;
            }
        }

        ed::PopStyleVar(2);

        ImGui::Spring();

        ImGui::EndVertical();

        ImGui::Spring(1);

        ImGui::BeginVertical("outputs", ImVec2(0, 0), 1.0f);

        ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
        ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0, 0));

        for (uint32_t group = 0; group < node->flowOutGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->flowOutCount(group); ++index) {
                FlowOutPin(node->flowOutName(index, group).data(), nodeId, index, group, node->flowOutMask(index, group), static_cast<bool>(node->flowOutTarget(index, group)));
            }

            if (dragPin && dragPin->mType == NodeGraph::PinType::Flow && (node->flowOutVariadic(group) && dragPin->mDir == NodeGraph::PinDir::In)) {
                uint32_t index = node->flowOutCount(group);
                FlowOutPin(node->flowOutName(index).data(), nodeId, index, group, node->flowOutMask(index, group), false);
            }
        }

        for (uint32_t group = 0; group < node->dataOutGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->dataOutCount(group); ++index) {
                NodeGraph::Pin source = node->dataOutTarget(index, group);

                ExtendedValueTypeDesc type = node->dataOutType(index, group);

                if (DataOutPin(node->dataOutName(index, group).data(), nodeId, index, group, type, node->dataOutMask(index, group), static_cast<bool>(source)))
                    hoveredPin = type;
            }

            if (dragPin && dragPin->mType == NodeGraph::PinType::DataInstance && node->dataOutVariadic(group) && dragPin->mDir == NodeGraph::PinDir::In) {
                uint32_t index = node->dataOutCount(group);
                if (DataOutPin(node->dataOutName(index).data(), nodeId, index, group, *dragType, node->dataOutMask(index, group), {}))
                    hoveredPin = *dragType;
            }
        }

        for (uint32_t group = 0; group < node->dataProviderGroupCount(); ++group) {
            for (uint32_t index = 0; index < node->dataProviderCount(group); ++index) {
                ExtendedValueTypeDesc type = node->dataProviderType(index, group);

                if (DataProviderPin(node->dataProviderName(index, group).data(), nodeId, index, group, type, node->dataProviderMask(index, group), !node->dataProviderTargets(index, group).empty()))
                    hoveredPin = type;
            }

            if (dragPin && dragPin->mType == NodeGraph::PinType::Data && node->dataProviderVariadic(group) && dragPin->mDir == NodeGraph::PinDir::Out) {
                uint32_t index = node->dataProviderCount(group);
                if (DataProviderPin(node->dataProviderName(index, group).data(), nodeId, index, group, *dragType, node->dataProviderMask(index, group), false))
                    hoveredPin = *dragType;
            }
        }

        ed::PopStyleVar(2);

        ImGui::Spring();

        ImGui::EndVertical();

        ImGui::EndHorizontal();

        return hoveredPin;
    }

    void EndNode()
    {

        ImGui::EndVertical();

        ImGui::PopID();

        ed::EndNode();
    }

    void NodeLinks(const NodeGraph::NodeBase *node, uint32_t nodeId)
    {
        for (uint32_t group = 0; group < node->flowOutGroupCount(); ++group) {
            for (uint32_t flowIndex = 0; flowIndex < node->flowOutCount(group); ++flowIndex) {
                if (NodeGraph::Pin target = node->flowOutTarget(flowIndex, group)) {
                    uint32_t id = 60000 * nodeId + NodeGraph::NodeBase::flowOutId(flowIndex, group);
                    ed::Link(id, id, 60000 * target.mNode + NodeGraph::NodeBase::flowInId(target.mIndex, target.mGroup), FlowColor(node->flowOutMask(flowIndex, group)));
                }
            }
        }
        for (uint32_t group = 0; group < node->dataInGroupCount(); ++group) {
            for (uint32_t dataIndex = 0; dataIndex < node->dataInCount(group); ++dataIndex) {
                if (NodeGraph::Pin source = node->dataInSource(dataIndex, group)) {
                    uint32_t id = 60000 * nodeId + NodeGraph::NodeBase::dataInId(dataIndex, group);
                    ed::Link(id, 60000 * source.mNode + NodeGraph::NodeBase::dataProviderId(source.mIndex, source.mGroup), id, DataColor(node->dataInMask(dataIndex, group)));
                }
            }
        }
        for (uint32_t group = 0; group < node->dataOutGroupCount(); ++group) {
            for (uint32_t dataIndex = 0; dataIndex < node->dataOutCount(group); ++dataIndex) {
                if (NodeGraph::Pin target = node->dataOutTarget(dataIndex, group)) {
                    uint32_t id = 60000 * nodeId + NodeGraph::NodeBase::dataOutId(dataIndex, group);
                    ed::Link(id, id, 60000 * target.mNode + NodeGraph::NodeBase::dataReceiverId(target.mIndex, target.mGroup), DataColor(node->dataOutMask(dataIndex, group)));
                }
            }
        }
    }

    void DrawIcon(ImDrawList *drawList, const ImVec2 &a, const ImVec2 &b, IconType type, bool filled, ImU32 color, ImU32 innerColor)
    {
        auto rect = ImRect(a, b);
        auto rect_x = rect.Min.x;
        auto rect_y = rect.Min.y;
        auto rect_w = rect.Max.x - rect.Min.x;
        auto rect_h = rect.Max.y - rect.Min.y;
        auto rect_center_x = (rect.Min.x + rect.Max.x) * 0.5f;
        auto rect_center_y = (rect.Min.y + rect.Max.y) * 0.5f;
        auto rect_center = ImVec2(rect_center_x, rect_center_y);
        const auto outline_scale = rect_w / 24.0f;
        const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

        if (type == IconType::Flow) {
            const auto origin_scale = rect_w / 24.0f;

            const auto offset_x = 1.0f * origin_scale;
            const auto offset_y = 0.0f * origin_scale;
            const auto margin = (filled ? 2.0f : 2.0f) * origin_scale;
            const auto rounding = 0.1f * origin_scale;
            const auto tip_round = 0.7f; // percentage of triangle edge (for tip)
            //const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
            const auto canvas = ImRect(
                rect.Min.x + margin + offset_x,
                rect.Min.y + margin + offset_y,
                rect.Max.x - margin + offset_x,
                rect.Max.y - margin + offset_y);
            const auto canvas_x = canvas.Min.x;
            const auto canvas_y = canvas.Min.y;
            const auto canvas_w = canvas.Max.x - canvas.Min.x;
            const auto canvas_h = canvas.Max.y - canvas.Min.y;

            const auto left = canvas_x + canvas_w * 0.5f * 0.3f;
            const auto right = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
            const auto top = canvas_y + canvas_h * 0.5f * 0.2f;
            const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
            const auto center_y = (top + bottom) * 0.5f;
            //const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

            const auto tip_top = ImVec2(canvas_x + canvas_w * 0.5f, top);
            const auto tip_right = ImVec2(right, center_y);
            const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

            drawList->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
            drawList->PathBezierCubicCurveTo(
                ImVec2(left, top),
                ImVec2(left, top),
                ImVec2(left, top) + ImVec2(rounding, 0));
            drawList->PathLineTo(tip_top);
            drawList->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
            drawList->PathBezierCubicCurveTo(
                tip_right,
                tip_right,
                tip_bottom + (tip_right - tip_bottom) * tip_round);
            drawList->PathLineTo(tip_bottom);
            drawList->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
            drawList->PathBezierCubicCurveTo(
                ImVec2(left, bottom),
                ImVec2(left, bottom),
                ImVec2(left, bottom) - ImVec2(0, rounding));

            if (!filled) {
                if (innerColor & 0xFF000000)
                    drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

                drawList->PathStroke(color, true, 2.0f * outline_scale);
            } else
                drawList->PathFillConvex(color);
        } else {
            auto triangleStart = rect_center_x + 0.32f * rect_w;

            auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

            rect.Min.x += rect_offset;
            rect.Max.x += rect_offset;
            rect_x += rect_offset;
            rect_center_x += rect_offset * 0.5f;
            rect_center.x += rect_offset * 0.5f;

            if (type == IconType::Circle) {
                const auto c = rect_center;

                if (!filled) {
                    const auto r = 0.5f * rect_w / 2.0f - 0.5f;

                    if (innerColor & 0xFF000000)
                        drawList->AddCircleFilled(c, r, innerColor, 12 + extra_segments);
                    drawList->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
                } else {
                    drawList->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
                }
            }

            if (type == IconType::Square) {
                if (filled) {
                    const auto r = 0.5f * rect_w / 2.0f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

#if IMGUI_VERSION_NUM > 18101
                    drawList->AddRectFilled(p0, p1, color, 0, ImDrawFlags_RoundCornersAll);
#else
                    drawList->AddRectFilled(p0, p1, color, 0, 15);
#endif
                } else {
                    const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

                    if (innerColor & 0xFF000000) {
#if IMGUI_VERSION_NUM > 18101
                        drawList->AddRectFilled(p0, p1, innerColor, 0, ImDrawFlags_RoundCornersAll);
#else
                        drawList->AddRectFilled(p0, p1, innerColor, 0, 15);
#endif
                    }

#if IMGUI_VERSION_NUM > 18101
                    drawList->AddRect(p0, p1, color, 0, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
#else
                    drawList->AddRect(p0, p1, color, 0, 15, 2.0f * outline_scale);
#endif
                }
            }

            if (type == IconType::Grid) {
                const auto r = 0.5f * rect_w / 2.0f;
                const auto w = ceilf(r / 3.0f);

                const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
                const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

                auto tl = baseTl;
                auto br = baseBr;
                for (int i = 0; i < 3; ++i) {
                    tl.x = baseTl.x;
                    br.x = baseBr.x;
                    drawList->AddRectFilled(tl, br, color);
                    tl.x += w * 2;
                    br.x += w * 2;
                    if (i != 1 || filled)
                        drawList->AddRectFilled(tl, br, color);
                    tl.x += w * 2;
                    br.x += w * 2;
                    drawList->AddRectFilled(tl, br, color);

                    tl.y += w * 2;
                    br.y += w * 2;
                }

                triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
            }

            if (type == IconType::RoundSquare) {
                if (filled) {
                    const auto r = 0.5f * rect_w / 2.0f;
                    const auto cr = r * 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

#if IMGUI_VERSION_NUM > 18101
                    drawList->AddRectFilled(p0, p1, color, cr, ImDrawFlags_RoundCornersAll);
#else
                    drawList->AddRectFilled(p0, p1, color, cr, 15);
#endif
                } else {
                    const auto r = 0.5f * rect_w / 2.0f - 0.5f;
                    const auto cr = r * 0.5f;
                    const auto p0 = rect_center - ImVec2(r, r);
                    const auto p1 = rect_center + ImVec2(r, r);

                    if (innerColor & 0xFF000000) {
#if IMGUI_VERSION_NUM > 18101
                        drawList->AddRectFilled(p0, p1, innerColor, cr, ImDrawFlags_RoundCornersAll);
#else
                        drawList->AddRectFilled(p0, p1, innerColor, cr, 15);
#endif
                    }

#if IMGUI_VERSION_NUM > 18101
                    drawList->AddRect(p0, p1, color, cr, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
#else
                    drawList->AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
#endif
                }
            } else if (type == IconType::Diamond) {
                if (filled) {
                    const auto r = 0.607f * rect_w / 2.0f;
                    const auto c = rect_center;

                    drawList->PathLineTo(c + ImVec2(0, -r));
                    drawList->PathLineTo(c + ImVec2(r, 0));
                    drawList->PathLineTo(c + ImVec2(0, r));
                    drawList->PathLineTo(c + ImVec2(-r, 0));
                    drawList->PathFillConvex(color);
                } else {
                    const auto r = 0.607f * rect_w / 2.0f - 0.5f;
                    const auto c = rect_center;

                    drawList->PathLineTo(c + ImVec2(0, -r));
                    drawList->PathLineTo(c + ImVec2(r, 0));
                    drawList->PathLineTo(c + ImVec2(0, r));
                    drawList->PathLineTo(c + ImVec2(-r, 0));

                    if (innerColor & 0xFF000000)
                        drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

                    drawList->PathStroke(color, true, 2.0f * outline_scale);
                }
            } else {
                const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

                drawList->AddTriangleFilled(
                    ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                    ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                    ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                    color);
            }
        }
    }

    void Icon(const ImVec2 &size, IconType type, bool filled, const ImVec4 &color /* = ImVec4(1, 1, 1, 1)*/, const ImVec4 &innerColor /* = ImVec4(0, 0, 0, 0)*/)
    {
        if (ImGui::IsRectVisible(size)) {
            auto cursorPos = ImGui::GetCursorScreenPos();
            auto drawList = ImGui::GetWindowDrawList();
            DrawIcon(drawList, cursorPos, cursorPos + size, type, filled, ImColor(color), ImColor(innerColor));
        }

        ImGui::Dummy(size);
    }

}
}