#pragma once

#include "Generic/coroutines/generator.h"

#include "Interfaces/filesystem/path.h"

#include "Meta/keyvalue/valuetype_forward.h"

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

IMGUI_API bool DragMatrix3(const char *label, Engine::Matrix3 *m, float v_speed);
IMGUI_API bool DragMatrix3(const char *label, Engine::Matrix3 *m, float *v_speeds);
IMGUI_API bool DragMatrix4(const char *label, Engine::Matrix4 *m, float v_speed);
IMGUI_API bool DragMatrix4(const char *label, Engine::Matrix4 *m, float *v_speeds);

IMGUI_API void BeginGroupPanel(const char *name = "", const ImVec2 &size = ImVec2(0.0f, 0.0f), ImU32 backgroundColor = 0);
IMGUI_API void EndGroupPanel();

IMGUI_API bool BeginPopupCompoundContextItem(const char *str_id = nullptr);
IMGUI_API bool BeginPopupCompoundContextWindow(const char *str_id = nullptr, ImGuiPopupFlags popup_flags = 0);

IMGUI_API bool BeginStatus();
IMGUI_API void EndStatus();

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

//////// ValueType

struct IMGUI_API ValueTypeDrawer {
    static bool draw(Engine::ScopePtr &scope);
    static bool draw(const Engine::ScopePtr &scope);
    static bool draw(bool &b);
    static bool draw(const bool &b);
    static bool draw(Engine::CoWString &s);
    static bool draw(const Engine::CoWString &s);
    static bool draw(int &i);
    static bool draw(const int &i);
    static bool draw(uint64_t &i);
    static bool draw(const uint64_t &i);
    static bool draw(float &f);
    static bool draw(const float &f);
    static bool draw(Engine::Matrix3 &m);
    static bool draw(const Engine::Matrix3 &m);
    static bool draw(Engine::Matrix3 *m);
    static bool draw(const Engine::Matrix3 *m);
    static bool draw(Engine::Matrix4 &m);
    static bool draw(const Engine::Matrix4 &m);
    static bool draw(Engine::Matrix4 *m);
    static bool draw(const Engine::Matrix4 *m);
    static bool draw(Engine::Vector2 &v);
    static bool draw(const Engine::Vector2 &v);
    static bool draw(Engine::Vector3 &v);
    static bool draw(const Engine::Vector3 &v);
    static bool draw(Engine::Vector4 &v);
    static bool draw(const Engine::Vector4 &v);
    static bool draw(Engine::Vector2i &v);
    static bool draw(const Engine::Vector2i &v);
    static bool draw(Engine::Vector3i &v);
    static bool draw(const Engine::Vector3i &v);
    static bool draw(Engine::Vector4i &v);
    static bool draw(const Engine::Vector4i &v);
    static bool draw(Engine::KeyValueVirtualSequenceRange &range);
    static bool draw(const Engine::KeyValueVirtualSequenceRange &range);
    static bool draw(Engine::KeyValueVirtualAssociativeRange &range);
    static bool draw(const Engine::KeyValueVirtualAssociativeRange &range);
    static bool draw(Engine::KeyValueFunction &m);
    static bool draw(const Engine::KeyValueFunction &m);
    static bool draw(Engine::ApiFunction &m);
    static bool draw(const Engine::ApiFunction &m);
    static bool draw(Engine::BoundApiFunction &m);
    static bool draw(const Engine::BoundApiFunction &m);
    static bool draw(std::monostate &);
    static bool draw(const std::monostate &);
    static bool draw(Engine::Quaternion &q);
    static bool draw(const Engine::Quaternion &q);
    static bool draw(Engine::ObjectPtr &o);
    static bool draw(const Engine::ObjectPtr &o);
    static bool draw(Engine::Filesystem::Path &p);
    static bool draw(const Engine::Filesystem::Path &p);
    static bool draw(Engine::EnumHolder &e);
    static bool draw(const Engine::EnumHolder &e);
    static bool draw(Engine::FlagsHolder &f);
    static bool draw(const Engine::FlagsHolder &f);
    static bool draw(Engine::Color3 &c);
    static bool draw(const Engine::Color3 &c);
    static bool draw(Engine::Color4 &c);
    static bool draw(const Engine::Color4 &c);
    static bool draw(const Engine::KeyValueSender &s);
    static bool draw(const Engine::KeyValueBinding &b);
    template <typename Rep, typename Ratio>
    static bool draw(std::chrono::duration<Rep, Ratio> &d)
    {
        std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::duration<Rep, Ratio>>(d);
        if (draw(ns)) {
            d = std::chrono::duration_cast<std::chrono::nanoseconds>(ns);
            return true;
        }
        return false;
    }
    static bool draw(std::chrono::nanoseconds &d);
    static bool draw(const std::chrono::nanoseconds &d);
    static bool draw(Engine::ExtendedValueTypeDesc &t);
    static bool draw(const Engine::ExtendedValueTypeDesc &t);
};

IMGUI_API bool ScopeTypePicker(const Engine::MetaTable *&t);
IMGUI_API bool ValueTypeTypePicker(Engine::ValueTypeDesc &t);
IMGUI_API bool ValueTypeTypePicker(Engine::ExtendedValueTypeDesc &t);

IMGUI_API bool MethodPicker(const char *label, const std::vector<std::pair<std::string, Engine::BoundApiFunction>> &methods, Engine::BoundApiFunction *m, std::string *currentName, std::string *filter = nullptr, int expectedArgumentCount = -1);

////////// Drag & Drop ValueTypes

IMGUI_API void ResetDraggableValueType();

IMGUI_API void DraggableValueTypeSourceEx(std::string_view name, Engine::CallableView<void(Engine::ValueType &)> out, ImGuiDragDropFlags flags = 0);
template <typename T>
void DraggableValueTypeSource(std::string_view name, const T &data, ImGuiDragDropFlags flags = 0)
{
    DraggableValueTypeSourceEx(name, [&](Engine::ValueType &retVal) { Engine::to_ValueType(retVal, data); }, flags);
}

IMGUI_API bool AcceptDraggableValueType(Engine::CallableView<Engine::KeyValueResult(Engine::ValueType &, const Engine::ValueType &, bool)> output);
template <typename T, typename Validator = bool (*)(const T &)>
bool AcceptDraggableValueType(
    T &result, Validator &&validate = [](const T &t) { return true; })
{
    return AcceptDraggableValueType([&](Engine::ValueType &ret, const Engine::ValueType &v, bool set) { return ValueType_unwrap(ret, [&](const T &t) { if (set) result = t; return validate(t); }, v); });
}

IMGUI_API bool IsDraggableValueTypeBeingAccepted(Engine::CallableView<Engine::KeyValueResult(const Engine::ValueType &)> output);
template <typename T>
bool IsDraggableValueTypeBeingAccepted(
    T &result)
{
    return IsDraggableValueTypeBeingAccepted([&](const Engine::ValueType &v) {
        return ValueType_unwrap([&](const T &t) { result = t; }, v);
    });
}

///////// Filepicker

struct FilesystemPickerOptions {
    const char *(*mIconLookup)(const Engine::Filesystem::Path &path, bool isDir) = nullptr;
    Engine::Filesystem::Path mBase;
    std::vector<std::string> mExtensions;
};

IMGUI_API FilesystemPickerOptions *GetFilesystemPickerOptions();

IMGUI_API bool DirectoryPicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, const FilesystemPickerOptions &options = {});
IMGUI_API bool FilePicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, bool *itemDoubleClicked = nullptr, const FilesystemPickerOptions &options = {});

///////// Interactive View

struct InteractiveViewState {
    bool mMouseDown[3] = { false, false, false };
    bool mDragging[3] = { false, false, false };
    bool mMouseClicked[3] = { false, false, false };
    bool mActive = false;
};

IMGUI_API bool InteractiveView(InteractiveViewState &state);

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
