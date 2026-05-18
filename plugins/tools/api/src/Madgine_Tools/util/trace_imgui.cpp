#include "../toolslib.h"

#include "trace_imgui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

namespace ImGui {

bool Checkbox(const char *label, const Engine::Tools::Traced<bool *> &v)
{
    bool changed = Checkbox(label, v.get());
    if (changed) {
        (*v).track(!(*v).get());
    }
    return changed;
}

bool ColorEdit3(const char *label, const Engine::Tools::Traced<float *> &col, ImGuiColorEditFlags flags)
{
    auto deref = col.trace([](float *p) { return std::array { p[0], p[1], p[2] }; });
    std::array<float, 3> value = deref.get();
    bool changed = ColorEdit3(label, col.get(), flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool ColorEdit4(const char *label, const Engine::Tools::Traced<float *> &col, ImGuiColorEditFlags flags)
{
    auto deref = col.trace([](float *p) { return std::array { p[0], p[1], p[2], p[3] }; });
    std::array<float, 4> value = deref.get();
    bool changed = ColorEdit4(label, col.get(), flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragFloat(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed, float v_min, float v_max, const char *format, ImGuiSliderFlags flags)
{
    float value = *v.get();
    bool changed = DragFloat(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        (*v).trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        (*v).submitContinuous();
    }
    return changed;
}

bool DragFloat2(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed, float v_min, float v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](float *p) { return std::array { p[0], p[1] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<float *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<float *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
        }
        return modified;
    }));
    std::array<float, 2> value = deref.get();
    bool changed = DragFloat2(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragFloat3(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed, float v_min, float v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](float *p) { return std::array { p[0], p[1], p[2] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<float *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<float *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
            array.mParent.get()[2] = array.get()[2];
        }
        return modified;
    }));
    std::array<float, 3> value = deref.get();
    bool changed = DragFloat3(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragFloat4(const char *label, const Engine::Tools::Traced<float *> &v, float v_speed, float v_min, float v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](float *p) { return std::array { p[0], p[1], p[2], p[3] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<float *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<float *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
            array.mParent.get()[2] = array.get()[2];
            array.mParent.get()[3] = array.get()[3];
        }
        return modified;
    }));
    std::array<float, 4> value = deref.get();
    bool changed = DragFloat4(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragInt(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
    int value = *v.get();
    bool changed = DragInt(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        (*v).trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        (*v).submitContinuous();
    }
    return changed;
}

bool DragInt2(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](int *p) { return std::array { p[0], p[1] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<int *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<int *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
        }
        return modified;
    }));
    std::array<int, 2> value = deref.get();
    bool changed = DragInt2(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragInt3(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](int *p) { return std::array { p[0], p[1], p[2] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<int *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<int *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
            array.mParent.get()[2] = array.get()[2];
        }
        return modified;
    }));
    std::array<int, 3> value = deref.get();
    bool changed = DragInt3(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragInt4(const char *label, const Engine::Tools::Traced<int *> &v, float v_speed, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
    auto f = [](int *p) { return std::array { p[0], p[1], p[2], p[3] }; };
    auto deref = v.traceEx(std::move(f), static_cast<bool (*)(const Engine::Tools::TracedAccess<int *, decltype(f)> &, bool)>([](const Engine::Tools::TracedAccess<int *, decltype(f)> &array, bool modified) {
        if (modified) {
            array.mParent.get()[0] = array.get()[0];
            array.mParent.get()[1] = array.get()[1];
            array.mParent.get()[2] = array.get()[2];
            array.mParent.get()[3] = array.get()[3];
        }
        return modified;
    }));
    std::array<int, 4> value = deref.get();
    bool changed = DragInt4(label, v.get(), v_speed, v_min, v_max, format, flags);
    if (changed) {
        deref.trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        deref.submitContinuous();
    }
    return changed;
}

bool DragDuration(const char *label, const Engine::Tools::Traced<std::chrono::nanoseconds::rep *> &p_data, float v_speed, const void *p_min, const void *p_max, const char *format, ImGuiSliderFlags flags)
{
    long long value = *p_data.get();
    bool changed = DragScalar(label, ImGuiDataType_S64, p_data.get(), v_speed, p_min, p_max, format, flags);
    if (changed) {
        (*p_data).trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        (*p_data).submitContinuous();
    }
    return changed;
}

bool DragUInt(const char *label, const Engine::Tools::Traced<uint64_t *> &p_data, float v_speed, const void *p_min, const void *p_max, const char *format, ImGuiSliderFlags flags)
{
    long long value = *p_data.get();
    bool changed = DragScalar(label, ImGuiDataType_U64, p_data.get(), v_speed, p_min, p_max, format, flags);
    if (changed) {
        (*p_data).trackContinuous(value);
    }
    if (IsItemDeactivatedAfterEdit()) {
        (*p_data).submitContinuous();
    }
    return changed;
}

bool InputText(const char *label, const Engine::Tools::Traced<Engine::CoWString *> &buf, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data)
{
    std::string value = *buf.get();
    bool changed = InputText(label, buf.get(), flags, callback, user_data);
    if (changed) {
        (*buf).trackContinuous(std::move(value));
    }
    if (IsItemDeactivatedAfterEdit()) {
        (*buf).submitContinuous();
    }
    return changed;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::ScopePtr &> &scope)
{
    Text("<scope>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::ScopePtr &> &scope)
{
    Text("<scope>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<bool &> &b)
{
    return Checkbox("##ValueTypeDrawer", &b);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const bool &> &b)
{
    BeginDisabled();
    auto ptr = &b;
    Checkbox("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<bool *> &>(ptr));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::CoWString &> &s)
{
    return InputText("##ValueTypeDrawer", &s, 0);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::CoWString &> &s)
{
    Text(s.get());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<int &> &i)
{
    PushItemWidth(100);
    return DragInt("##ValueTypeDrawer", &i);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const int &> &i)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = &i;
    DragInt("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<int *> &>(ptr));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<uint64_t &> &i)
{
    PushItemWidth(100);
    return DragUInt("##ValueTypeDrawer", &i, 1.0f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const uint64_t &> &i)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = &i;
    DragUInt("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<uint64_t *> &>(ptr), 1.0f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<float &> &f)
{
    PushItemWidth(100);
    return DragFloat("##ValueTypeDrawer", &f, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const float &> &f)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = &f;
    DragFloat("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::CoW<Engine::Matrix3> &> &m)
{
    return DragMatrix3("##ValueTypeDrawer", &m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Matrix3> &> &m)
{
    BeginDisabled();
    auto ptr = &m;
    DragMatrix3("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<Engine::Matrix3 *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Matrix3 *> &m)
{
    return DragMatrix3("##ValueTypeDrawer", m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Matrix3 *> &m)
{
    BeginDisabled();
    DragMatrix3("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<Engine::Matrix3 *> &>(m), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::CoW<Engine::Matrix4> &> &m)
{
    return DragMatrix4("##ValueTypeDrawer", &m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::CoW<Engine::Matrix4> &> &m)
{
    BeginDisabled();
    auto ptr = &m;
    DragMatrix4("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<Engine::Matrix4 *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Matrix4 *> &m)
{
    return DragMatrix4("##ValueTypeDrawer", m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Matrix4 *> &m)
{
    BeginDisabled();
    DragMatrix4("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<Engine::Matrix4 *> &>(m), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector2 &> &v)
{
    PushItemWidth(100);
    return DragFloat2("##ValueTypeDrawer", v.trace(&Engine::Vector2::ptr), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector2 &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector2::ptr);
    DragFloat2("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector3 &> &v)
{
    PushItemWidth(100);
    return DragFloat3("##ValueTypeDrawer", v.trace(&Engine::Vector3::ptr), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector3 &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector3::ptr);
    DragFloat3("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector4 &> &v)
{
    PushItemWidth(100);
    return DragFloat4("##ValueTypeDrawer", v.trace(&Engine::Vector4::ptr), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector4 &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector4::ptr);
    DragFloat4("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector2i &> &v)
{
    PushItemWidth(100);
    return DragInt2("##ValueTypeDrawer", v.trace(&Engine::Vector2i::ptr));
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector2i &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector2i::ptr);
    DragInt2("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<int *> &>(ptr));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector3i &> &v)
{
    PushItemWidth(100);
    return DragInt3("##ValueTypeDrawer", v.trace(&Engine::Vector3i::ptr));
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector3i &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector3i::ptr);
    DragInt3("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<int *> &>(ptr));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Vector4i &> &v)
{
    PushItemWidth(100);
    return DragInt4("##ValueTypeDrawer", v.trace(&Engine::Vector4i::ptr));
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Vector4i &> &v)
{
    BeginDisabled();
    PushItemWidth(100);
    auto ptr = v.trace(&Engine::Vector4i::ptr);
    DragInt4("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<int *> &>(ptr));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::KeyValueVirtualSequenceRange &> &it)
{
    Text("<range>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueVirtualSequenceRange &> &it)
{
    Text("<range>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::KeyValueVirtualAssociativeRange &> &it)
{
    Text("<map>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueVirtualAssociativeRange &> &it)
{
    Text("<map>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::KeyValueFunction &> &m)
{
    Text("<function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueFunction &> &m)
{
    Text("<function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::ApiFunction &> &m)
{
    Text("<api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::ApiFunction &> &m)
{
    Text("<api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::BoundApiFunction &> &m)
{
    Text("<bound api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::BoundApiFunction &> &m)
{
    Text("<bound api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<std::monostate &> &)
{
    Text("<null>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const std::monostate &> &)
{
    Text("<null>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Quaternion &> &q)
{
    const Engine::Tools::Traced<Engine::Vector3 &> &v = q.traceEx(&Engine::Quaternion::toDegrees, static_cast<bool (*)(const Engine::Tools::TracedAccess<Engine::Quaternion &, decltype(&Engine::Quaternion::toDegrees)> &, bool)>([](const Engine::Tools::TracedAccess<Engine::Quaternion &, decltype(&Engine::Quaternion::toDegrees)> &degrees, bool changed) {
        if (changed)
            degrees.mParent.get() = Engine::Quaternion::FromDegrees(degrees.get());
        return changed;
    }));

    return draw(v);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Quaternion &> &q)
{
    const Engine::Vector3 v = q->toDegrees();
    return draw(static_cast<const Engine::Tools::Traced<const Engine::Vector3 &> &>(Engine::Tools::TracedRoot { q.undoStack(), v }));
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::ObjectPtr &> &o)
{
    Text(o.get().descriptor());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::ObjectPtr &> &o)
{
    Text(o.get().descriptor());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Filesystem::Path &> &p)
{
    Text(p.get());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Filesystem::Path &> &p)
{
    Text(p.get());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::EnumHolder &> &e)
{
    bool changed = false;
    std::string name { e->toString() };
    if (ImGui::BeginCombo("##ValueTypeDrawer", name.c_str())) {
        for (int32_t i : e->table()->values<int32_t>()) {
            bool isSelected = e->value() == i;
            std::string valueName { e->table()->toString(i) };
            if (ImGui::Selectable(valueName.c_str(), isSelected)) {
                e->setValue(i);
                changed = true;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::EnumHolder &> &e)
{
    Text(e->toString());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::FlagsHolder &> &f)
{
    bool changed = false;
    std::string name { "<flags>" };
    if (ImGui::BeginCombo("##ValueTypeDrawer", name.c_str())) {
        for (int32_t i : f->table()->values<int32_t>()) {
            bool selected = f.get()[i];
            std::string valueName { f->table()->toString(i) };
            if (ImGui::Checkbox(valueName.c_str(), &selected)) {
                f.get()[i] = selected;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::FlagsHolder &> &f)
{
    Text("<flags>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Color3 &> &c)
{
    return ColorEdit3("##ValueTypeDrawer", &c.trace(&Engine::Color3::r), ImGuiColorEditFlags_NoInputs);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Color3 &> &c)
{
    auto ptr = &c.trace(&Engine::Color3::r);
    return ColorEdit3("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::Color4 &> &c)
{
    return ColorEdit4("##ValueTypeDrawer", &c.trace(&Engine::Color4::r), ImGuiColorEditFlags_NoInputs);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::Color4 &> &c)
{
    auto ptr = &c.trace(&Engine::Color4::r);
    return ColorEdit4("##ValueTypeDrawer", reinterpret_cast<const Engine::Tools::Traced<float *> &>(ptr), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueSender &> &s)
{
    Text("<sender>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueBinding &> &b)
{
    Text("<binding>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::KeyValueScopeBinding &> &b)
{
    Text("<binding>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<std::chrono::nanoseconds &> &d)
{
    auto count = d.traceEx(&std::chrono::nanoseconds::count, static_cast<bool (*)(const Engine::Tools::TracedAccess<std::chrono::nanoseconds &, decltype(&std::chrono::nanoseconds::count)> &, bool)>([](const auto &count, bool modified) {
        if (modified) {
            count.mParent.get() = std::chrono::nanoseconds { count.get() };
        }
        return modified;
    }));
    if (ImGui::DragDuration("##ValueTypeDrawer", &count, 100000000.0f)) {
        d.get() = std::chrono::nanoseconds { count.get() };
        return true;
    }
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const std::chrono::nanoseconds &> &d)
{
    Text("<duration>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<Engine::ExtendedValueTypeDesc &> &t)
{
    return ValueTypeTypePicker(t.get());
}

bool ValueTypeDrawer::draw(const Engine::Tools::Traced<const Engine::ExtendedValueTypeDesc &> &t)
{
    Text(t->toString());
    return false;
}

bool ScopeTypePicker(const Engine::MetaTable *&t)
{
    const Engine::MetaTable *type = Engine::sTypeList();
    while (type) {
        if (Selectable(type->mTypeName)) {
            t = type;
            return true;
        }
        type = type->mNext;
    }
    return false;
}

template <typename Ty, typename T>
bool SelectValueTypeType(T &t)
{
    T desc = Engine::toValueTypeDesc<Ty>();

    bool result = false;

    if constexpr (std::same_as<Ty, Engine::OwnedScopePtr>) {
        if (ImGui::BeginMenu("OwnedScopePtr")) {
            const Engine::MetaTable *type;
            result = ScopeTypePicker(type);
            if (result)
                desc = Engine::ValueTypeDesc { Engine::ValueTypeEnum::OwnedScopeValue, type->mSelf };
            ImGui::EndMenu();
        }
    } else if constexpr (std::same_as<Ty, Engine::ScopePtr>) {
        if (ImGui::BeginMenu("ScopePtr")) {
            const Engine::MetaTable *type;
            result = ScopeTypePicker(type);
            if (result)
                desc = Engine::ValueTypeDesc { Engine::ValueTypeEnum::ScopeValue, type->mSelf };
            ImGui::EndMenu();
        }
    } else {
        result = Selectable(desc.toString().data(), t == desc);
    }
    if (result)
        t = desc;
    return result;
}

template <typename... Ty, typename T>
bool SelectValueTypeTypes(Engine::type_pack<Ty...>, T &t)
{
    return (SelectValueTypeType<Ty>(t) || ...);
}

bool ValueTypeTypePicker(Engine::ValueTypeDesc &t)
{
    bool changed = false;
    if (ImGui::BeginCombo("##combo", "", ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft)) {
        changed |= SelectValueTypeTypes(Engine::ValueTypeList::transform<Engine::type_pack_first> {}, t);
        ImGui::EndCombo();
    }
    return changed;
}

bool ValueTypeTypePicker(Engine::ExtendedValueTypeDesc &t)
{
    bool changed = false;
    if (ImGui::BeginCombo("##combo", "", ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft)) {
        changed |= SelectValueTypeTypes(Engine::ValueTypeList::transform<Engine::type_pack_first> {}, t);
        ImGui::EndCombo();
    }
    return changed;
}

bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Matrix3 *> &m, float *v_speeds)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    bool value_changed = false;

    BeginGroup();
    PushID(label);

    for (int i = 0; i < 3; ++i) {
        PushMultiItemsWidths(3, std::min(300.0f, CalcItemWidth()));
        for (int j = 0; j < 3; ++j) {
            PushID(3 * i + j);
            if (j > 0) {
                SameLine(0, g.Style.ItemInnerSpacing.x);
            }
            value_changed |= DragFloat("", &(*m)[i][j], v_speeds[3 * i + j]);
            PopItemWidth();
            PopID();
        }
        if (i == 0) {
            SameLine(0, g.Style.ItemInnerSpacing.x);
            const char *end = FindRenderedTextEnd(label);
            TextEx(label, end);
        }
    }
    PopID();
    EndGroup();

    return value_changed;
}

bool DragMatrix3(const char *label, const Engine::Tools::Traced<Engine::Matrix3 *> &m, float v_speed)
{
    float speeds[9];
    std::fill_n(speeds, 9, v_speed);
    return DragMatrix3(label, m, speeds);
}

bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Matrix4 *> &m, float *v_speeds)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    bool value_changed = false;

    const char *end = FindRenderedTextEnd(label);
    TextEx(label, end);
    SameLine();

    BeginGroup();
    PushID(label);

    for (int i = 0; i < 4; ++i) {
        PushMultiItemsWidths(4, std::min(400.0f, CalcItemWidth()));
        for (int j = 0; j < 4; ++j) {
            PushID(4 * i + j);
            value_changed |= DragFloat("", &(*m)[i][j], v_speeds[4 * i + j]);
            PopItemWidth();
            PopID();
            if (j < 3) {
                SameLine(0, g.Style.ItemInnerSpacing.x);
            }
        }
    }
    PopID();
    EndGroup();

    return value_changed;
}

bool DragMatrix4(const char *label, const Engine::Tools::Traced<Engine::Matrix4 *> &m, float v_speed)
{
    float speeds[16];
    std::fill_n(speeds, 16, v_speed);
    return DragMatrix4(label, m, speeds);
}

bool MethodPicker(const char *label, const std::vector<std::pair<std::string, Engine::BoundApiFunction>> &methods, Engine::BoundApiFunction *m, std::string *currentName, std::string *filter, int expectedArgumentCount)
{
    bool result = false;

    if (!label)
        label = "##testid";

    std::string current;
    if (m->mScope)
        current = m->scope().name() + ("." + *currentName);
    if (ImGui::BeginCombo(label, current.c_str())) {
        if (filter)
            ImGui::InputText("filter", filter);
        for (auto &[name, method] : methods) {
            if (!filter) {
                if (expectedArgumentCount == -1 || method.argumentsCount() == expectedArgumentCount) {
                    bool is_selected = (method == *m);
                    std::string fullItemName = method.scope().name() + ("." + name);
                    if (ImGui::Selectable(fullItemName.c_str(), is_selected)) {
                        *currentName = name;
                        *m = method;
                        result = true;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }
    return result;
}

static ValueTypePayload sPayload;

void ResetDraggableValueType()
{
    sPayload.mValue = {};
}

void DraggableValueTypeSourceEx(std::string_view name, const Engine::Tools::Traced<const Engine::ValueType &> &value, ImGuiDragDropFlags flags)
{
    if (ImGui::BeginDragDropSource(flags | ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
        ValueTypePayload *payload = &sPayload;
        if (!ImGui::GetDragDropPayload()) {
            payload->mName = name;
            payload->mValue = value.build();
            ImGui::SetDragDropPayload("ValueType", &payload, sizeof(payload), ImGuiCond_Once);
        }
        ImGui::EndDragDropSource();
    }
}

const ValueTypePayload *GetValuetypePayload()
{
    const ImGuiPayload *payload = ImGui::GetDragDropPayload();

    if (payload && payload->IsDataType("ValueType")) {
        return *static_cast<const ValueTypePayload **>(payload->Data);
    }

    return nullptr;
}

bool AcceptDraggableValueType(Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> output, Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> validate)
{
    const ValueTypePayload *payload = GetValuetypePayload();
    if (payload) {
        Engine::KeyValueResult error = payload->mValue([&](const Engine::Tools::Traced<const Engine::ValueType &> &v) { return std::make_pair(validate(v), false); });
        if (!error) {
            error = payload->mValue([&](const Engine::Tools::Traced<const Engine::ValueType &> &v) { return std::make_pair(output(v), false); });
        }
        if (error) {
            if (ImGui::BeginTooltip()) {
                ImGui::TextColored(ImColor(255, 40, 40, 255), "%s", error.mError->mMsg.c_str());
                ImGui::EndTooltip();
            }
        } else {
            return ImGui::AcceptDragDropPayload("ValueType");
        }
    }
    return false;
}

bool IsDraggableValueTypeBeingAccepted(Engine::CallableView<Engine::KeyValueResult(const Engine::Tools::Traced<const Engine::ValueType &> &)> output)
{
    if (IsDragDropPayloadBeingAccepted()) {
        const ValueTypePayload *payload = GetValuetypePayload();
        if (!payload)
            return false;
        Engine::KeyValueResult error = payload->mValue([&](const Engine::Tools::Traced<const Engine::ValueType &> &v) { return std::make_pair(output(v), false); });
        if (error) {
        } else {
            return true;
        }
    }

    return false;
}
}