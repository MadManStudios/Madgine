#pragma once

#include "Generic/containers/generator.h"

#include "Platform/filesystem/path.h"

#include "Meta/reflect/util.h"

#include "Modules/debug/history.h"

#include "imconfig.h"
#include "imgui.h"

namespace ImGui {    

//////// Utility

IMGUI_API void Text(std::string_view s);
IMGUI_API bool InputText(const char *label, std::string *s, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr);
IMGUI_API bool InputText(const char *label, Engine::CoWString *s, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void *user_data = nullptr);

IMGUI_API void BeginTreeArrow(const void *label, ImGuiTreeNodeFlags flags = 0);
IMGUI_API bool EndTreeArrow(bool *opened = nullptr);
IMGUI_API void BeginSpanningTreeNode(const void *id, const char *label, ImGuiTreeNodeFlags flags = 0);
IMGUI_API bool EndSpanningTreeNode();
IMGUI_API bool EditableTreeNode(const void *id, std::string *s, ImGuiTreeNodeFlags flags = 0);

IMGUI_API void Duration(std::chrono::nanoseconds dur);
IMGUI_API void RightAlignDuration(std::chrono::nanoseconds dur);

IMGUI_API void Bytes(size_t bytes);
IMGUI_API void RightAlignBytes(size_t bytes);

IMGUI_API void RightAlignText(const char *s, ...);
IMGUI_API void RightAlign(float size);

IMGUI_API void BeginGroupPanel(const char *name = "", const ImVec2 &size = ImVec2(0.0f, 0.0f), ImU32 backgroundColor = 0);
IMGUI_API void EndGroupPanel();

IMGUI_API bool BeginPopupCompoundContextItem(const char *str_id = nullptr);
IMGUI_API bool BeginPopupCompoundContextWindow(const char *str_id = nullptr, ImGuiPopupFlags popup_flags = 0);

IMGUI_API bool BeginStatus();
IMGUI_API void EndStatus();

IMGUI_API bool BeginToolBar(const char *name);
IMGUI_API void EndToolBar();

IMGUI_API bool Spinner(const char *label, float radius, int thickness, const ImU32 &color);
IMGUI_API void DrawSpinner(const ImVec2 &min, const ImVec2 &max, float radius, int thickness, const ImU32 &color);

using Unit = std::pair<size_t, const char *>;

static constexpr Unit sByteUnits[] {
    { 1024, "B" },
    { 1024, "KB" },
    { 1024, "MB" },
    { 1024, "GB" }
};

static constexpr Unit sDurationUnits[] {
    { 1000, "ns" },
    { 1000, "us" },
    { 1000, "ms" },
    { 60, "s" }
};

IMGUI_API void UnitText(float value, std::span<const Unit> units, void (*text)(const char *, ...) = ImGui::Text);

template <typename E>
bool EnumCombo(const char *name, E *val)
{
    bool changed = false;
    if (ImGui::BeginCombo(name, std::string { val->toString() }.c_str())) {
        for (E v : E::values()) {
            if (ImGui::Selectable(std::string { v.toString() }.c_str())) {
                *val = v;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

IMGUI_API bool LED(const char *label, bool on, const ImVec2 &size = ImVec2(12, 12));
IMGUI_API bool LED(const char *label, bool *on, const ImVec2 &size = ImVec2(12, 12));

IMGUI_API bool InlineContextButton(const char *text, bool checked = false);

///////// Filepicker

struct FilesystemPickerOptions {
    const char *(*mIconLookup)(const Engine::Platform::Filesystem::Path &path, bool isDir) = nullptr;
    Engine::Platform::Filesystem::Path mBase;
    std::vector<std::string> mExtensions;
};

IMGUI_API FilesystemPickerOptions *GetFilesystemPickerOptions();

IMGUI_API bool DirectoryPicker(Engine::Platform::Filesystem::Path &path, Engine::Platform::Filesystem::Path &selection, const FilesystemPickerOptions &options = {});
IMGUI_API bool FilePicker(Engine::Platform::Filesystem::Path &path, Engine::Platform::Filesystem::Path &selection, bool *itemDoubleClicked = nullptr, const FilesystemPickerOptions &options = {});

///////// Interactive View

IMGUI_API bool WasMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold);

enum InteractiveViewResultFlags_ {
    InteractiveViewResultFlags_NoMouseButton = 0,
    InteractiveViewResultFlags_LeftMouseButton = 1,
    InteractiveViewResultFlags_MiddleMouseButton = 2,
    InteractiveViewResultFlags_RightMouseButton = 3,
    InteractiveViewResultFlags_MouseButtonMask_ = 0x3,
    InteractiveViewResultFlags_Active = (1 << 2),
    InteractiveViewResultFlags_Pressed = (1 << 3),
    InteractiveViewResultFlags_DragStarted = (1 << 4),
    InteractiveViewResultFlags_DragStopped = (1 << 5),
    InteractiveViewResultFlags_Dragging = (1 << 6),
    InteractiveViewResultFlags_Hovered = (1 << 7)
};
typedef int InteractiveViewResultFlags;

IMGUI_API InteractiveViewResultFlags InteractiveView();
IMGUI_API InteractiveViewResultFlags InteractiveView(ImGuiID id);
IMGUI_API bool BeginPopupContextInteractiveView(ImGuiPopupFlags flags = 0);

/////////// Docking

IMGUI_API void SetWindowDockingDir(ImGuiID dockSpaceId, ImGuiDir dir, float ratio, bool outer, ImGuiCond cond = 0);

IMGUI_API void MakeTabVisible(const char *name);

////////// History

template <size_t S>
void PlotHistory(Engine::Debug::History<float, S> &data, const char *label, std::span<const Unit> units = {})
{
    if (ImGui::BeginTable(label, 3)) {
        ImGui::TableSetupColumn("plot", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("statNames", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("stats", ImGuiTableColumnFlags_WidthFixed);
        const Engine::Debug::HistoryData<float> &d = data.data();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        float range = d.mMax - d.mMin;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PlotHistogram("", data.buffer(), S, static_cast<int>(d.mIndex), nullptr, d.mMin - 0.05f * range, d.mMax + 0.05f * range, ImVec2(0, 80));
        ImGui::TableNextColumn();
        ImGui::Text("Average: ");
        ImGui::Text("Current: ");
        ImGui::Text("Min: ");
        ImGui::Text("Max: ");
        ImGui::TableNextColumn();
        ImGui::UnitText(data.average(), units, ImGui::RightAlignText);
        ImGui::UnitText(data.buffer()[(d.mIndex + S - 1) % S], units, ImGui::RightAlignText);
        ImGui::UnitText(d.mMin, units, ImGui::RightAlignText);
        ImGui::UnitText(d.mMax, units, ImGui::RightAlignText);

        /* if (ImGui::Button("Reset extreme values"))
        data.resetExtremeValues();*/
        ImGui::EndTable();
    }
}

}
