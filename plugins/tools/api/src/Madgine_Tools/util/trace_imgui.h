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

MADGINE_TOOLS_EXPORT bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Math::Matrix3 *> &m, float v_speed);
MADGINE_TOOLS_EXPORT bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Math::Matrix3 *> &m, float *v_speeds);
MADGINE_TOOLS_EXPORT bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Math::Matrix4 *> &m, float v_speed);
MADGINE_TOOLS_EXPORT bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Math::Matrix4 *> &m, float *v_speeds);

//////// ValueType

struct MADGINE_TOOLS_EXPORT ValueTypeDrawer {
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::ScopePtr &> &scope);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::ScopePtr &> &scope);
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
    static bool draw(const Engine::Tools::Traced<Engine::CoW<Engine::Math::Matrix3> &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Math::Matrix3> &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Matrix3 *> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Matrix3 *> &m);
    static bool draw(const Engine::Tools::Traced<Engine::CoW<Engine::Math::Matrix4> &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Math::Matrix4> &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Matrix4 *> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Matrix4 *> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector2 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector2 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector3 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector3 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector4 &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector4 &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector2i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector2i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector3i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector3i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Vector4i &> &v);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Vector4i &> &v);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::SequenceRange &> &range);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::SequenceRange &> &range);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::AssociativeRange &> &range);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::AssociativeRange &> &range);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::Function &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::Function &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::ApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::ApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::BoundApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::BoundApiFunction &> &m);
    static bool draw(const Engine::Tools::Traced<std::monostate &> &);
    static bool draw(const Engine::Tools::Traced<const std::monostate &> &);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Quaternion &> &q);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Quaternion &> &q);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::ObjectPtr &> &o);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::ObjectPtr &> &o);
    static bool draw(const Engine::Tools::Traced<Engine::Platform::Filesystem::Path &> &p);
    static bool draw(const Engine::Tools::Traced<const Engine::Platform::Filesystem::Path &> &p);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::Enum &> &e);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::Enum &> &e);
    static bool draw(const Engine::Tools::Traced<Engine::Reflect::Flags &> &f);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::Flags &> &f);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Color3 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Color3 &> &c);
    static bool draw(const Engine::Tools::Traced<Engine::Math::Color4 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::Math::Color4 &> &c);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::Sender &> &s);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::Binding &> &b);
    static bool draw(const Engine::Tools::Traced<const Engine::Reflect::ScopeBinding &> &b);
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
};

MADGINE_TOOLS_EXPORT bool TypeIterate(Engine::CallableView<bool(const Engine::TypeName &)> visitor);

MADGINE_TOOLS_EXPORT bool ScopeTypePicker(const Engine::Reflect::MetaTable *&t);
MADGINE_TOOLS_EXPORT bool ValueTypeTypePicker(Engine::Reflect::Type &t);
MADGINE_TOOLS_EXPORT bool ValueTypeTypePicker(Engine::Reflect::ExtendedType &t);

MADGINE_TOOLS_EXPORT bool MethodPicker(const char *label, const std::vector<std::pair<std::string, Engine::Reflect::BoundApiFunction>> &methods, Engine::Reflect::BoundApiFunction *m, std::string *currentName, std::string *filter = nullptr, int expectedArgumentCount = -1);

////////// Drag & Drop ValueTypes

struct ValueTypePayload {
    std::string mName;
    Engine::Closure<Engine::Reflect::Result(Engine::CallableView<std::pair<Engine::Reflect::Result, bool>(const Engine::Tools::Traced<const Engine::Reflect::Value &> &)>)> mValue;
};

MADGINE_TOOLS_EXPORT void ResetDraggableValueType();

MADGINE_TOOLS_EXPORT void DraggableValueTypeSourceEx(std::string_view name, const Engine::Tools::Traced<const Engine::Reflect::Value &> &value, ImGuiDragDropFlags flags = 0);
template <typename T>
void DraggableValueTypeSource(std::string_view name, const Engine::Tools::Traced<const T &> &data, ImGuiDragDropFlags flags = 0)
{
    DraggableValueTypeSourceEx(name, Engine::Reflect::toValue(data), flags);
}

MADGINE_TOOLS_EXPORT bool AcceptDraggableValueType(Engine::CallableView<Engine::Reflect::Result(const Engine::Tools::Traced<const Engine::Reflect::Value &> &)> output, Engine::CallableView<Engine::Reflect::Result(const Engine::Tools::Traced<const Engine::Reflect::Value &> &)> validate);
template <typename T, typename Validator = Engine::Reflect::Result (*)(const Engine::Tools::Traced<const T &> &)>
bool AcceptDraggableValueType(
    T &result, Validator &&validate = [](const Engine::Tools::Traced<const T &> &t) { return Engine::Reflect::Result {}; })
{
    return AcceptDraggableValueType(
        [&](const Engine::Tools::Traced<const Engine::Reflect::Value &> &v) { return call([&](const Engine::Tools::Traced<const T &> &t) { result = t.get(); return Engine::Reflect::Result{}; }, v); },
        [&](const Engine::Tools::Traced<const Engine::Reflect::Value &> &v) { return call([&](const Engine::Tools::Traced<const T &> &t) { return validate(t); }, v); });
}

MADGINE_TOOLS_EXPORT bool IsDraggableValueTypeBeingAccepted(Engine::CallableView<Engine::Reflect::Result(const Engine::Tools::Traced<const Engine::Reflect::Value &> &)> output);
template <typename T>
bool IsDraggableValueTypeBeingAccepted(T &result)
{
    return IsDraggableValueTypeBeingAccepted([&](const Engine::Tools::Traced<const Engine::Reflect::Value &> &v) {
        return call([&](const Engine::Tools::Traced<const T &> &t) { result = t.get(); return Engine::Reflect::Result{}; }, v);
    });
}
}