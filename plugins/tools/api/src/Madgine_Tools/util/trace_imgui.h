#pragma once

#include "imgui/imgui.h"
#include "trace.h"

namespace ImGui {

////////// Traced Widgets

MADGINE_TOOLS_EXPORT bool Checkbox(const char *label, const Engine::Tools::Traced<bool *> &v);
MADGINE_TOOLS_EXPORT bool ColorEdit3(const char *label, const Engine::Tools::Traced<float *> &col, ImGuiColorEditFlags flags = 0);
MADGINE_TOOLS_EXPORT bool ColorEdit4(const char *label, const Engine::Tools::Traced<float *> &col, ImGuiColorEditFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragFloat(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f", ImGuiSliderFlags flags = 0); // If v_min >= v_max we have no bound
MADGINE_TOOLS_EXPORT bool DragFloat2(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragFloat3(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragFloat4(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragInt(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char *format = "%d", ImGuiSliderFlags flags = 0); // If v_min >= v_max we have no bound
MADGINE_TOOLS_EXPORT bool DragInt2(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char *format = "%d", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragInt3(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char *format = "%d", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragInt4(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char *format = "%d", ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragDuration(const char *label, const Engine::Tools::Traced<std::chrono::nanoseconds::rep *> &p_data, float v_speed = 1.0f, const void *p_min = NULL, const void *p_max = NULL, const char *format = NULL, ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool DragUInt(const char *label, const Engine::Tools::Traced<uint64_t *> &p_data, float v_speed = 1.0f, const void *p_min = NULL, const void *p_max = NULL, const char *format = NULL, ImGuiSliderFlags flags = 0);
MADGINE_TOOLS_EXPORT bool InputText(const char *label, const Engine::Tools::Traced<Engine::CoWString *> &buf, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void *user_data = NULL);

MADGINE_TOOLS_EXPORT bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Matrix3 *> &m, float v_speed);
MADGINE_TOOLS_EXPORT bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Matrix3 *> &m, float *v_speeds);
MADGINE_TOOLS_EXPORT bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Matrix4 *> &m, float v_speed);
MADGINE_TOOLS_EXPORT bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Matrix4 *> &m, float *v_speeds);

//////// ValueType

struct MADGINE_TOOLS_EXPORT ValueTypeDrawer {
    static bool draw(const Engine::Tools::Traced<Engine::ScopePtr &> &scope);
    static bool draw(const Engine::Tools::Traced<const Engine::ScopePtr &> &scope);
    static bool draw(const Engine::Tools::Traced<bool &> &b);
    static bool draw(const Engine::Tools::Traced<const bool &> &b);
    static bool draw(const Engine::Tools::Traced<Engine::CoWString &> &s);
    static bool draw(const Engine::Tools::Traced<const Engine::CoWString &> &s);
    static bool draw(const Engine::Tools::Traced<int &> &i);
    static bool draw(const Engine::Tools::Traced<const int &> &i);
    static bool draw(const Engine::Tools::Traced<uint64_t &> &i);
    static bool draw(const Engine::Tools::Traced<const uint64_t &> &i);
    static bool draw(const Engine::Tools::Traced<float &> &f);
    static bool draw(const Engine::Tools::Traced<const float &> &f);
    static bool draw(const Engine::Tools::Traced<Engine::CoW<Engine::Matrix3> &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Matrix3> &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Matrix3 *> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Matrix3 *> &m);
    static bool draw(const Engine::Tools::Traced<Engine::CoW<Engine::Matrix4> &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Matrix4> &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Matrix4 *> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Matrix4 *> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Vector2 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector2 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Vector3 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector3 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Vector4 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector4 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Vector2i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector2i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Vector3i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector3i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Vector4i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Vector4i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::KeyValueVirtualSequenceRange &> &range);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueVirtualSequenceRange &> &range);
    static bool draw(const Engine::Tools::Traced<Engine::KeyValueVirtualAssociativeRange &> &range);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueVirtualAssociativeRange &> &range);
    static bool draw(const Engine::Tools::Traced<Engine::KeyValueFunction &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueFunction &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::ApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::ApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::BoundApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::BoundApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<std::monostate &> &);
    static bool draw(const Engine::Tools::Traced<const std::monostate &> &);
    static bool draw(const Engine::Tools::Traced<Engine::Quaternion &> &q);
    static bool draw(const Engine::Tools::Traced<const Engine::Quaternion &> &q);
    static bool draw(const Engine::Tools::Traced<Engine::ObjectPtr &> &o);
    static bool draw(const Engine::Tools::Traced<const Engine::ObjectPtr &> &o);
    static bool draw(const Engine::Tools::Traced<Engine::Filesystem::Path &> &p);
    static bool draw(const Engine::Tools::Traced<const Engine::Filesystem::Path &> &p);
    static bool draw(const Engine::Tools::Traced<Engine::EnumHolder &> &e);
    static bool draw(const Engine::Tools::Traced<const Engine::EnumHolder &> &e);
    static bool draw(const Engine::Tools::Traced<Engine::FlagsHolder &> &f);
    static bool draw(const Engine::Tools::Traced<const Engine::FlagsHolder &> &f);
    static bool draw(const Engine::Tools::Traced<Engine::Color3 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::Color3 &> &c);
    static bool draw(const Engine::Tools::Traced<Engine::Color4 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::Color4 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueSender &> &s);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueBinding &> &b);
    static bool draw(const Engine::Tools::Traced<const Engine::KeyValueScopeBinding &> &b);
    template <typename Rep, typename Ratio>
    static bool draw(const Engine::Tools::Traced<std::chrono::duration<Rep, Ratio> &> &d)
    {
        auto ns = d.traceEx(&std::chrono::duration_cast<std::chrono::nanoseconds, Rep, Ratio>, static_cast<bool (*)(const Engine::Tools::TracedAccess<std::chrono::duration<Rep, Ratio> &, decltype(&std::chrono::duration_cast<std::chrono::nanoseconds, Rep, Ratio>)> &, bool)>([](const auto &dur, bool modified) {
            if (modified) {
                dur.mParent.get() = std::chrono::duration_cast<std::chrono::duration<Rep, Ratio>>(dur.get());
            }
            return modified;
        }));
        if (draw(static_cast<const Engine::Tools::Traced<std::chrono::nanoseconds &> &>(ns))) {
            d.get() = std::chrono::duration_cast<std::chrono::duration<Rep, Ratio>>(ns.get());
            return true;
        }
        return false;
    }
    static bool draw(const Engine::Tools::Traced<std::chrono::nanoseconds &> &d);
    static bool draw(const Engine::Tools::Traced<const std::chrono::nanoseconds &> &d);
    static bool draw(const Engine::Tools::Traced<Engine::ExtendedValueTypeDesc &> &t);
    static bool draw(const Engine::Tools::Traced<const Engine::ExtendedValueTypeDesc &> &t);
};

MADGINE_TOOLS_EXPORT bool ScopeTypePicker(const Engine::MetaTable *&t);
MADGINE_TOOLS_EXPORT bool ValueTypeTypePicker(Engine::ValueTypeDesc &t);
MADGINE_TOOLS_EXPORT bool ValueTypeTypePicker(Engine::ExtendedValueTypeDesc &t);

MADGINE_TOOLS_EXPORT bool MethodPicker(const char *label, const std::vector<std::pair<std::string, Engine::BoundApiFunction>> &methods, Engine::BoundApiFunction *m, std::string *currentName, std::string *filter = nullptr, int expectedArgumentCount = -1);

////////// Drag & Drop ValueTypes

struct ValueTypePayload {
    std::string mName;
    Engine::Closure<Engine::KeyValueResult(Engine::CallableView<std::pair<Engine::KeyValueResult, bool>(const Engine::Tools::Traced<const Engine::ValueType &> &)>)> mValue;
};

MADGINE_TOOLS_EXPORT void ResetDraggableValueType();

MADGINE_TOOLS_EXPORT void DraggableValueTypeSourceEx(std::string_view name, const Engine::Tools::Traced<const Engine::ValueType &> &value, ImGuiDragDropFlags flags = 0);
template <typename T>
void DraggableValueTypeSource(std::string_view name, const Engine::Tools::Traced<const T &> &data, ImGuiDragDropFlags flags = 0)
{
    DraggableValueTypeSourceEx(name, Engine::to_ValueType(data), flags);
}

MADGINE_TOOLS_EXPORT bool AcceptDraggableValueType(Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> output, Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> validate);
template <typename T, typename Validator = Engine::KeyValueResult (*)(const Engine::Tools::Traced<const T &> &)>
bool AcceptDraggableValueType(
    T &result, Validator &&validate = [](const Engine::Tools::Traced<const T &> &t) { return Engine::KeyValueResult {}; })
{
    return AcceptDraggableValueType(
        [&](const Engine::Tools::Traced<const Engine::ValueType &> &v) { return ValueType_call([&](const Engine::Tools::Traced<const T &> &t) { result = t.get(); return Engine::KeyValueResult{}; }, v); },
        [&](const Engine::Tools::Traced<const Engine::ValueType &> &v) { return ValueType_call([&](const Engine::Tools::Traced<const T &> &t) { return validate(t); }, v); });
}

MADGINE_TOOLS_EXPORT bool IsDraggableValueTypeBeingAccepted(Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> output);
template <typename T>
bool IsDraggableValueTypeBeingAccepted(T &result)
{
    return IsDraggableValueTypeBeingAccepted([&](const Engine::Tools::Traced<const Engine::ValueType &> &v) {
        return ValueType_call([&](const Engine::Tools::Traced<const T &> &t) { result = t.get(); return Engine::KeyValueResult{}; }, v);
    });
}
}