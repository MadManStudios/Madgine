#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiaddons.h"


#include "../imgui/imgui_internal.h"

#include "Meta/math/matrix3.h"

#include "Meta/keyvalue/scopeptr.h"

#include "Meta/keyvalue/boundapifunction.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/filesystem/path.h"

#include "valuetypepayload.h"

#include "Generic/enum.h"

#include "Modules/threading/workgroupstorage.h"

#include "Generic/coroutines/generator.h"

#include "Modules/threading/threadlocal.h"

//Engine::Threading::WorkgroupLocal<ImGuiContext *>
THREADLOCAL(ImGuiContext *)
sContext;

ImGuiContext *&getImGuiContext()
{
    return sContext;
}

namespace ImGui {

static Engine::Threading::WorkgroupLocal<ImVector<ImRect>> sGroupPanelLabelStack;

static Engine::Threading::WorkgroupLocal<FilesystemPickerOptions> sFilesystemPickerOptions;

static ValueTypePayload sPayload;

bool ValueTypeDrawer::draw(Engine::ScopePtr &scope)
{
    Text("<scope>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::ScopePtr &scope)
{
    Text("<scope>");
    return false;
}

bool ValueTypeDrawer::draw(bool &b)
{
    return Checkbox("##ValueTypeDrawer", &b);
}

bool ValueTypeDrawer::draw(const bool &b)
{
    BeginDisabled();
    Checkbox("##ValueTypeDrawer", const_cast<bool *>(&b));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::CoWString &s)
{
    return InputText("##ValueTypeDrawer", &s, 0);
}

bool ValueTypeDrawer::draw(const Engine::CoWString &s)
{
    Text(s);
    return false;
}

bool ValueTypeDrawer::draw(int &i)
{
    PushItemWidth(100);
    return DragInt("##ValueTypeDrawer", &i);
}

bool ValueTypeDrawer::draw(const int &i)
{
    BeginDisabled();
    PushItemWidth(100);
    DragInt("##ValueTypeDrawer", const_cast<int *>(&i));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(uint64_t &i)
{
    PushItemWidth(100);
    return DragScalar("##ValueTypeDrawer", ImGuiDataType_U64, &i, 1.0f);
}

bool ValueTypeDrawer::draw(const uint64_t &i)
{
    BeginDisabled();
    PushItemWidth(100);
    DragScalar("##ValueTypeDrawer", ImGuiDataType_U64, const_cast<uint64_t *>(&i), 1.0f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(float &f)
{
    PushItemWidth(100);
    return DragFloat("##ValueTypeDrawer", &f, 0.15f);
}

bool ValueTypeDrawer::draw(const float &f)
{
    BeginDisabled();
    PushItemWidth(100);
    DragFloat("##ValueTypeDrawer", const_cast<float *>(&f), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Matrix3 &m)
{
    return DragMatrix3("##ValueTypeDrawer", &m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Matrix3 &m)
{
    BeginDisabled();
    DragMatrix3("##ValueTypeDrawer", const_cast<Engine::Matrix3 *>(&m), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Matrix3 *m)
{
    return draw(*m);
}

bool ValueTypeDrawer::draw(const Engine::Matrix3 *m)
{
    return draw(*m);
}

bool ValueTypeDrawer::draw(Engine::Matrix4 &m)
{
    return DragMatrix4("##ValueTypeDrawer", &m, 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Matrix4 &m)
{
    BeginDisabled();
    DragMatrix4("##ValueTypeDrawer", const_cast<Engine::Matrix4 *>(&m), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Matrix4 *m)
{
    return draw(*m);
}

bool ValueTypeDrawer::draw(const Engine::Matrix4 *m)
{
    return draw(*m);
}

bool ValueTypeDrawer::draw(Engine::Vector2 &v)
{
    PushItemWidth(100);
    return DragFloat2("##ValueTypeDrawer", v.ptr(), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Vector2 &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragFloat2("##ValueTypeDrawer", const_cast<float *>(v.ptr()), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Vector3 &v)
{
    PushItemWidth(100);
    return DragFloat3("##ValueTypeDrawer", v.ptr(), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Vector3 &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragFloat3("##ValueTypeDrawer", const_cast<float *>(v.ptr()), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Vector4 &v)
{
    PushItemWidth(100);
    return DragFloat4("##ValueTypeDrawer", v.ptr(), 0.15f);
}

bool ValueTypeDrawer::draw(const Engine::Vector4 &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragFloat4("##ValueTypeDrawer", const_cast<float *>(v.ptr()), 0.15f);
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Vector2i &v)
{
    PushItemWidth(100);
    return DragInt2("##ValueTypeDrawer", v.ptr());
}

bool ValueTypeDrawer::draw(const Engine::Vector2i &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragInt2("##ValueTypeDrawer", const_cast<int *>(v.ptr()));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Vector3i &v)
{
    PushItemWidth(100);
    return DragInt3("##ValueTypeDrawer", v.ptr());
}

bool ValueTypeDrawer::draw(const Engine::Vector3i &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragInt3("##ValueTypeDrawer", const_cast<int *>(v.ptr()));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::Vector4i &v)
{
    PushItemWidth(100);
    return DragInt4("##ValueTypeDrawer", v.ptr());
}

bool ValueTypeDrawer::draw(const Engine::Vector4i &v)
{
    BeginDisabled();
    PushItemWidth(100);
    DragInt4("##ValueTypeDrawer", const_cast<int *>(v.ptr()));
    EndDisabled();
    return false;
}

bool ValueTypeDrawer::draw(Engine::KeyValueVirtualSequenceRange &it)
{
    Text("<range>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::KeyValueVirtualSequenceRange &it)
{
    Text("<range>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::KeyValueVirtualAssociativeRange &it)
{
    Text("<map>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::KeyValueVirtualAssociativeRange &it)
{
    Text("<map>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::KeyValueFunction &m)
{
    Text("<function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::KeyValueFunction &m)
{
    Text("<function>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::ApiFunction &m)
{
    Text("<api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::ApiFunction &m)
{
    Text("<api-function>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::BoundApiFunction &m)
{
    Text("<bound api-function>");
    return false;
}

bool ValueTypeDrawer::draw(const Engine::BoundApiFunction &m)
{
    Text("<bound api-function>");
    return false;
}

bool ValueTypeDrawer::draw(std::monostate &)
{
    Text("<null>");
    return false;
}

bool ValueTypeDrawer::draw(const std::monostate &)
{
    Text("<null>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::Quaternion &q)
{
    Engine::Vector3 v = q.toDegrees();

    if (draw(v)) {
        q = Engine::Quaternion::FromDegrees(v);
        return true;
    }
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Quaternion &q)
{
    const Engine::Vector3 v = q.toDegrees();
    return draw(v);
}

bool ValueTypeDrawer::draw(Engine::ObjectPtr &o)
{
    Text(o.descriptor());
    return false;
}

bool ValueTypeDrawer::draw(const Engine::ObjectPtr &o)
{
    Text(o.descriptor());
    return false;
}

bool ValueTypeDrawer::draw(Engine::Filesystem::Path &p)
{
    Text(p);
    return false;
}

bool ValueTypeDrawer::draw(const Engine::Filesystem::Path &p)
{
    Text(p);
    return false;
}

bool ValueTypeDrawer::draw(Engine::EnumHolder &e)
{
    bool changed = false;
    std::string name { e.toString() };
    if (ImGui::BeginCombo("##ValueTypeDrawer", name.c_str())) {
        for (int32_t i : e.table()->values<int32_t>()) {
            bool isSelected = e.value() == i;
            std::string valueName { e.table()->toString(i) };
            if (ImGui::Selectable(valueName.c_str(), isSelected)) {
                e.setValue(i);
                changed = true;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool ValueTypeDrawer::draw(const Engine::EnumHolder &e)
{
    Text(e.toString());
    return false;
}

bool ValueTypeDrawer::draw(Engine::FlagsHolder &f)
{
    bool changed = false;
    std::string name { "<flags>" };
    if (ImGui::BeginCombo("##ValueTypeDrawer", name.c_str())) {
        for (int32_t i : f.table()->values<int32_t>()) {
            bool selected = f[i];
            std::string valueName { f.table()->toString(i) };
            if (ImGui::Checkbox(valueName.c_str(), &selected)) {
                f[i] = selected;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool ValueTypeDrawer::draw(const Engine::FlagsHolder &f)
{
    Text("<flags>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::Color3 &c)
{
    return ColorEdit3("##ValueTypeDrawer", &c.r, ImGuiColorEditFlags_NoInputs);
}

bool ValueTypeDrawer::draw(const Engine::Color3 &c)
{
    return ColorEdit3("##ValueTypeDrawer", const_cast<float *>(&c.r), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
}

bool ValueTypeDrawer::draw(Engine::Color4 &c)
{
    return ColorEdit4("##ValueTypeDrawer", &c.r, ImGuiColorEditFlags_NoInputs);
}

bool ValueTypeDrawer::draw(const Engine::Color4 &c)
{
    return ColorEdit4("##ValueTypeDrawer", const_cast<float *>(&c.r), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
}

bool ValueTypeDrawer::draw(const Engine::KeyValueSender &s)
{
    Text("<sender>");
    return false;
}

bool ValueTypeDrawer::draw(std::chrono::nanoseconds &d)
{
    int64_t count = d.count();
    if (ImGui::DragScalar("##ValueTypeDrawer", ImGuiDataType_S64, &count, 100000000.0f)) {
        d = std::chrono::nanoseconds { count };
        return true;
    }
    return false;
}

bool ValueTypeDrawer::draw(const std::chrono::nanoseconds &d)
{
    Text("<duration>");
    return false;
}

bool ValueTypeDrawer::draw(Engine::ExtendedValueTypeDesc &t)
{
    return ValueTypeTypePicker(t);
}

bool ValueTypeDrawer::draw(const Engine::ExtendedValueTypeDesc &t)
{
    Text(t.toString());
    return false;
}

void setPayloadStatus(std::string_view msg)
{
    if (ImGui::GetIO().KeyShift)
        sPayload.mStatusMessage = msg;
}

void Text(std::string_view s)
{
    TextUnformatted(s.data(), s.data() + s.size());
}

template <typename S>
struct InputTextCallback_UserData {
    S &mString;
    ImGuiInputTextCallback mChainCallback = nullptr;
    void *mChainCallbackUserData = nullptr;
};

template <typename S>
static int InputTextCallback(ImGuiInputTextCallbackData *data)
{
    InputTextCallback_UserData<S> *user_data = (InputTextCallback_UserData<S> *)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        S &str = user_data->mString;
        IM_ASSERT(data->Buf == str.data());
        str.resize(data->BufTextLen);
        data->Buf = str.data();
    } else if (user_data->mChainCallback) {
        // Forward to user callback, if any
        data->UserData = user_data->mChainCallbackUserData;
        return user_data->mChainCallback(data);
    }
    return 0;
}

bool InputText(const char *label, std::string *s, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data)
{
    InputTextCallback_UserData<std::string> cb {
        *s,
        callback,
        user_data
    };

    return ImGui::InputText(label, s->data(), s->capacity() + 1, flags | ImGuiInputTextFlags_CallbackResize, &InputTextCallback<std::string>, &cb);
}

bool InputText(const char *label, Engine::CoWString *s, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void *user_data)
{
    InputTextCallback_UserData<Engine::CoWString> cb {
        *s,
        callback,
        user_data
    };

    return ImGui::InputText(label, s->data(), s->size() + 1, flags | ImGuiInputTextFlags_CallbackResize, &InputTextCallback<Engine::CoWString>, &cb);
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
    } else if constexpr (std::same_as<Ty, Engine::ScopePtr>){
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

void BeginTreeArrow(const void *label, ImGuiTreeNodeFlags flags)
{
    ImGui::PushID(label);
    ImGuiID id = ImGui::GetID("treeArrow");

    ImGuiStorage *storage = ImGui::GetStateStorage();
    bool *opened = storage->GetBoolRef(id);

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    ImGuiContext &g = *GImGui;
    const ImVec2 label_size = ImGui::CalcTextSize("asd", nullptr, false);
    const ImGuiStyle &style = g.Style;
    const float text_base_offset_y = ImMax(0.0f, window->DC.CurrLineTextBaseOffset); // Latch before ItemSize changes it
    const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y);
    ImRect frame_bb = ImRect(window->DC.CursorPos, ImVec2(window->Pos.x + ImGui::GetContentRegionMax().x, window->DC.CursorPos.y + frame_height));
    ItemSize(ImVec2(label_size.x, frame_height), text_base_offset_y);

    if (!(flags & ImGuiTreeNodeFlags_Leaf)) {
        const ImRect interact_bb = ImRect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + style.ItemSpacing.x * 2, frame_bb.Max.y);

        bool hovered = false;

        bool pressed = ImGui::ButtonBehavior(interact_bb, id, &hovered, nullptr, ImGuiButtonFlags_PressedOnClick);

        ImGui::RenderArrow(window->DrawList, ImVec2(style.FramePadding.x + frame_bb.Min.x, g.FontSize * 0.15f + text_base_offset_y + frame_bb.Min.y), ImColor(255, 255, 255), *opened || hovered ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);

        if (pressed)
            *opened = !*opened;
    }
}

bool EndTreeArrow(bool *opened)
{
    ImGuiID id = ImGui::GetID("treeArrow");
    if (!opened) {
        ImGuiStorage *storage = ImGui::GetStateStorage();
        opened = storage->GetBoolRef(id);
    }

    ImGui::PopID();

    if (*opened)
        ImGui::TreePushOverrideID(id);

    return *opened;
}

void BeginSpanningTreeNode(const void *id, const char *label, ImGuiTreeNodeFlags flags)
{

    BeginTreeArrow(id, flags);
    ImGui::SameLine(0.0f, 0.0f);

    bool b = ImGui::Selectable(label, false, ImGuiSelectableFlags_SpanAllColumns);

    if (b && !(flags & ImGuiTreeNodeFlags_Leaf)) {
        ImGuiStorage *storage = ImGui::GetStateStorage();
        bool *opened = storage->GetBoolRef(ImGui::GetID("treeArrow"));
        *opened = !*opened;
    }
}

bool EndSpanningTreeNode()
{
    return EndTreeArrow();
}

bool TempInputText(const ImRect &bb, ImGuiID id, const char *label, std::string *s, ImGuiInputTextFlags flags)
{
    // On the first frame, g.TempInputTextId == 0, then on subsequent frames it becomes == id.
    // We clear ActiveID on the first frame to allow the InputText() taking it back.
    ImGuiContext &g = *GImGui;
    const bool init = (g.TempInputId != id);
    if (init)
        ClearActiveID();

    g.CurrentWindow->DC.CursorPos = bb.Min;

    InputTextCallback_UserData<std::string> cb {
        *s
    };

    bool value_changed = InputTextEx(label, NULL, s->data(), s->size() + 1, bb.GetSize(), flags | ImGuiInputTextFlags_MergedItem | ImGuiInputTextFlags_CallbackResize, &InputTextCallback<std::string>, &cb);
    if (init) {
        // First frame we started displaying the InputText widget, we expect it to take the active id.
        IM_ASSERT(g.ActiveId == id);
        g.TempInputId = g.ActiveId;
    }
    return value_changed;
}

bool EditableTreeNode(const void *id, std::string *s, ImGuiTreeNodeFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;

    BeginTreeArrow(id, flags);
    ImGui::SameLine(0.0f, 0.0f);

    ImVec2 pos_before = window->DC.CursorPos;

    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(g.Style.ItemSpacing.x, g.Style.FramePadding.y * 2.0f));
    bool b = Selectable("##Selectable", flags & ImGuiTreeNodeFlags_Selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap);
    PopStyleVar();

    ImGuiID inputId = window->GetID("##Input");
    bool temp_input_is_active = TempInputIsActive(inputId);
    bool temp_input_start = b && IsMouseDoubleClicked(0);

    if (temp_input_start)
        SetActiveID(inputId, window);

    if (temp_input_is_active || temp_input_start) {
        ImVec2 pos_after = window->DC.CursorPos;
        window->DC.CursorPos = pos_before;

        TempInputText(g.LastItemData.Rect, inputId, "##Input", s, ImGuiInputTextFlags_None);

        ItemAdd(g.LastItemData.Rect, inputId);
        window->DC.CursorPos = pos_after;
    } else {
        window->DrawList->AddText(pos_before, GetColorU32(ImGuiCol_Text), s->c_str());
    }

    return EndTreeArrow();
}

void UnitText(float value, std::span<const Unit> units, void (*text)(const char *, ...))
{
    if (units.empty()) {
        text("%.2f", value);
    } else {
        for (size_t i = 0; i < units.size(); ++i) {
            auto [ratio, unit] = units[i];
            if (value < ratio || i == units.size() - 1) {
                text("%.2f %s", value, unit);
                break;
            } else {
                value /= ratio;
            }
        }
    }
}

void Duration(std::chrono::nanoseconds dur)
{
    UnitText(dur.count(), sDurationUnits, Text);
}

void RightAlignDuration(std::chrono::nanoseconds dur)
{
    UnitText(dur.count(), sDurationUnits, RightAlignText);
}

void Bytes(size_t bytes)
{
    UnitText(bytes, sByteUnits, Text);
}

void RightAlignBytes(size_t bytes)
{
    UnitText(bytes, sByteUnits, RightAlignText);
}

void RightAlign(float size)
{
    ImGuiWindow *window = GetCurrentWindow();
    float avail = ImGui::GetContentRegionAvail().x;
    //ImGui::Dummy({ 0, 0 });
    //ImGui::SameLine(avail - size);
    //window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + (avail - size) + window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
}

void RightAlignText(const char *s, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, s);
    int len = vsprintf(buffer, s, args);
    va_end(args);
    assert(len >= 0);

    float w = CalcTextSize(buffer, buffer + len).x;

    RightAlign(w);
    ImGui::Text("%s", buffer);
}

bool DragMatrix3(const char *label, Engine::Matrix3 *m, float *v_speeds)
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

bool DragMatrix3(const char *label, Engine::Matrix3 *m, float v_speed)
{
    float speeds[9];
    std::fill_n(speeds, 9, v_speed);
    return DragMatrix3(label, m, speeds);
}

bool DragMatrix4(const char *label, Engine::Matrix4 *m, float *v_speeds)
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

bool DragMatrix4(const char *label, Engine::Matrix4 *m, float v_speed)
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

void DraggableValueTypeSource(std::string_view name, void (*source)(Engine::ValueType &, void *), void *data, ImGuiDragDropFlags flags)
{
    if (ImGui::BeginDragDropSource(flags)) {
        ValueTypePayload *payload = &sPayload;
        payload->mName = name;
        source(payload->mValue, data);
        ImGui::SetDragDropPayload("ValueType", &payload, sizeof(payload), ImGuiCond_Once);
        ImGui::Text(name);
        ImGui::Text(payload->mValue.getTypeString());
        if (!payload->mStatusMessage.empty()) {
            ImGui::Text(payload->mStatusMessage);
            payload->mStatusMessage.clear();
        }
        ImGui::EndDragDropSource();
    }
}

const ValueTypePayload *GetValuetypePayloadData()
{
    const ImGuiPayload *payload = ImGui::GetDragDropPayload();

    if (payload && payload->IsDataType("ValueType")) {
        return *static_cast<const ValueTypePayload **>(payload->Data);
    }

    return nullptr;
}

const Engine::ValueType *GetValuetypePayload()
{
    const ValueTypePayload *payload = GetValuetypePayloadData();
    if (payload) {
        return &payload->mValue;
    }
    return nullptr;
}

bool AcceptDraggableValueType(
    Engine::ValueType &result, Engine::ExtendedValueTypeDesc type, const ValueTypePayload **payloadPointer, std::function<bool(const Engine::ValueType &)> validate)
{
    const Engine::ValueType *payload = GetValuetypePayload();
    if (payload) {
        if (validate(*payload) && type.canAccept(payload->type()) && AcceptDraggableValueType(payloadPointer)) {
            result = *payload;
            return true;
        }
    }
    return false;
}

bool AcceptDraggableValueType(const ValueTypePayload **payloadPointer)
{
    if (ImGui::AcceptDragDropPayload("ValueType")) {
        if (payloadPointer)
            *payloadPointer = GetValuetypePayloadData();
        return true;
    }
    return false;
}

bool IsDraggableValueTypeBeingAccepted(const ValueTypePayload **payloadPointer)
{
    ImGuiContext &g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow *window = g.CurrentWindow;
    if (!(g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect))
        return false;
    if (g.HoveredWindowUnderMovingWindow == NULL || window->RootWindow != g.HoveredWindowUnderMovingWindow->RootWindow)
        return false;

    const ImRect &display_rect = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) ? g.LastItemData.DisplayRect : g.LastItemData.Rect;
    ImGuiID id = g.LastItemData.ID;
    if (id == 0)
        id = window->GetIDFromRectangle(display_rect);
    if (g.DragDropPayload.SourceId == id)
        return false;

    if (g.DragDropAcceptIdPrev == id) {
        if (payloadPointer)
            *payloadPointer = GetValuetypePayloadData();
        return true;
    }
    return false;
}

bool BeginFilesystemPicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection)
{
    bool changed = false;

    if (path.empty())
        path = Engine::Filesystem::Path { "." }.absolute();
    if (selection.empty())
        selection = Engine::Filesystem::Path { "." }.absolute();

    if (ImGui::Button("Up")) {
        selection = path;
        path = path / "..";
        changed = true;
    }

    ImGui::SameLine();

    std::string s = path.str();

    if (ImGui::InputText("Current", &s)) {
        path = s;
        changed = true;
    }

    return changed;
}

bool DirectoryPicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, const FilesystemPickerOptions &options)
{
    bool changed = BeginFilesystemPicker(path, selection);

    if (ImGui::BeginChild("CurrentFolder", { 0.0f, -ImGui::GetFrameHeightWithSpacing() })) {

        for (Engine::Filesystem::FileQueryResult result : Engine::Filesystem::listDirs(path)) {

            bool selected = selection == result.path();

            if (ImGui::Selectable(result.path().filename().c_str(), selected)) {
                selection = result.path();
                changed = true;
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                path = result.path();
                selection = path;
                changed = true;
            }
        }
    }

    ImGui::EndChild();

    return changed;
}

FilesystemPickerOptions *GetFilesystemPickerOptions()
{
    return &sFilesystemPickerOptions;
}

bool FilePicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, bool *clicked, const FilesystemPickerOptions &options)
{
    bool changed = BeginFilesystemPicker(path, selection);

    bool selectedIsFile = false;
    bool selectedIsDir = false;

    if (ImGui::BeginTable("CurrentFolder", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        //ImGui::TableSetupColumn("Total time");
        //ImGui::TableSetupColumn("Rel. Time (parent)");
        //ImGui::TableSetupColumn("Rel. Time (total)");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        struct File {
            Engine::Filesystem::Path mPath;
            bool mIsDir;
        };
        std::vector<File> files;

        std::ranges::transform(Engine::Filesystem::listFilesAndDirs(path), std::back_inserter(files), [](Engine::Filesystem::FileQueryResult result) { return File { result.path(), result.isDir() }; });

        for (const File &file : files) {

            ImGui::TableNextRow();

            ImGui::TableNextColumn();

            bool selected = selection == file.mPath;
            if (selected) {
                if (file.mIsDir)
                    selectedIsDir = true;
                else
                    selectedIsFile = true;
            }

            auto iconLookup = options.mIconLookup ? options.mIconLookup : sFilesystemPickerOptions->mIconLookup;

            std::string name = file.mPath.filename();
            if (iconLookup) {
                name = iconLookup(file.mPath, file.mIsDir) + name;
            }

            if (ImGui::Selectable(name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                selection = file.mPath;
                changed = true;
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                changed = true;
                if (file.mIsDir) {
                    path = file.mPath;
                } else {
                    selection = file.mPath;
                    if (clicked)
                        *clicked = true;
                }
            }
        }

        ImGui::EndTable();
    }

    std::string fileName = selection.relative(path).str();
    if (InputText("Filename:", &fileName)) {
        Engine::Filesystem::Path p = fileName;
        if (!p.empty() && p.isRelative())
            selection = path / p;

        changed = true;
    }

    return changed;
}

bool InteractiveView(InteractiveViewState &state)
{
    state.mActive = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    ImGuiIO &io = ImGui::GetIO();

    if (state.mActive) {
        ImGui::SetItemUsingMouseWheel();
        for (int i = 0; i < 3; ++i) {
            if (io.MouseClicked[i]) {
                state.mMouseDown[i] = true;
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        state.mMouseClicked[i] = false;
        if (state.mMouseDown[i]) {

            if (Engine::Vector2 { io.MouseDelta }.length() >= io.MouseDragThreshold / 3.0f && !state.mDragging[0] && !state.mDragging[1] && !state.mDragging[2]) {
                state.mDragging[i] = true;
            }

            if (io.MouseReleased[i]) {
                state.mMouseDown[i] = false;
                if (!state.mDragging[i]) {
                    state.mMouseClicked[i] = true;
                }
                state.mDragging[i] = false;
            }
        }
    }

    return state.mActive;
}

void ImGui::BeginGroupPanel(const char *name, const ImVec2 &size)
{
    ImGui::BeginGroup();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    auto frameHeight = ImGui::GetFrameHeight();
    ImGui::BeginGroup();

    ImVec2 effectiveSize = size;
    if (size.x < 0.0f)
        effectiveSize.x = ImGui::GetContentRegionAvail().x;
    else
        effectiveSize.x = size.x;
    ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

    ImVec2 labelMin, labelMax;

    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::BeginGroup();
    if (strlen(name) > 0) {
        ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextUnformatted(name);
        labelMin = ImGui::GetItemRectMin();
        labelMax = ImGui::GetItemRectMax();
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
    } else {
        ImGui::Dummy(ImVec2(0.0, frameHeight * 0.5f + itemSpacing.y));
    }

    ImGui::BeginGroup();

    //ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x -= frameHeight;

    auto itemWidth = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

    if (strlen(name) > 0)
        sGroupPanelLabelStack->push_back(ImRect(labelMin, labelMax));
    else
        sGroupPanelLabelStack->push_back(ImRect());
}

void ImGui::EndGroupPanel()
{
    ImGui::PopItemWidth();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    auto frameHeight = ImGui::GetFrameHeight();

    ImGui::EndGroup();

    //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

    ImGui::EndGroup();

    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();
    //ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

    auto labelRect = sGroupPanelLabelStack->back();
    sGroupPanelLabelStack->pop_back();

    ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
    ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
    labelRect.Min.x -= itemSpacing.x;
    labelRect.Max.x += itemSpacing.x;
    for (int i = 0; i < 4; ++i) {
        switch (i) {
        // left half-plane
        case 0:
            ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true);
            break;
        // right half-plane
        case 1:
            ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true);
            break;
        // top
        case 2:
            ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true);
            break;
        // bottom
        case 3:
            ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true);
            break;
        }

        ImGui::GetWindowDrawList()->AddRect(
            frameRect.Min, frameRect.Max,
            ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
            halfFrame.x);

        ImGui::PopClipRect();
    }

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x += frameHeight;

    ImGui::Dummy(ImVec2(0.0f, 0.0f));

    ImGui::EndGroup();
}

bool BeginPopupCompoundContextItem(const char *str_id)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    if (!str_id)
        str_id = "compound_context";
    ImGuiID id = ImHashStr(str_id, 0, ImHashData(&window, sizeof(window)));

    ImRect rect { GetItemRectMin(), GetItemRectMax() };

    bool open = BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);

    if (open) {
        ImVec2 pos = GetMousePosOnOpeningCurrentPopup();
        if (rect.Contains(pos)) {
            ImGui::Separator();
        } else {
            open = false;
            EndPopup();
        }
    }

    return open;
}

bool BeginPopupCompoundContextWindow(const char *str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiWindow *window = GImGui->CurrentWindow;
    if (!str_id)
        str_id = "compound_context";
    ImGuiID id = ImHashStr(str_id, 0, ImHashData(&window, sizeof(window)));
    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool IsNewWindow(const char *name)
{
    return FindWindowByName(name) == nullptr;
}

void SetWindowDockingDir(ImGuiID dockSpaceId, ImGuiDir dir, float ratio, bool outer, ImGuiCond cond)
{
    ImGuiWindow *window = GetCurrentWindow();

    if (cond && (window->SetWindowDockAllowFlags & cond) == 0)
        return;

    window->SetWindowDockAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    if (dir == ImGuiDir_Down || dir == ImGuiDir_Right)
        ratio = 1.0f - ratio;

    ImGuiDockNode *node;
    ImGuiDockNode *centralNode = DockBuilderGetCentralNode(dockSpaceId);
    bool dock = false;

    ImGuiAxis axis = (dir == ImGuiDir_Left || dir == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;
    int dirIndex = (dir == ImGuiDir_Left || dir == ImGuiDir_Up) ? 0 : 1;

    if (outer) {
        node = DockBuilderGetNode(dockSpaceId);
        ImGuiDockNode *itNode = node;
        while (itNode->IsSplitNode()) {
            if (itNode->SplitAxis == axis) {
                ImGuiDockNode *upNode = centralNode;
                while (upNode && upNode->ParentNode != itNode->ChildNodes[0] && upNode->ParentNode != itNode->ChildNodes[1])
                    upNode = upNode->ParentNode;
                if (upNode) {
                    int itDirIndex = upNode->ParentNode == itNode->ChildNodes[0] ? 1 : 0;
                    if (itDirIndex == dirIndex) {
                        if (!itNode->ChildNodes[dirIndex]->IsSplitNode()) {
                            dir = ImGuiDir_None;
                            node = itNode->ChildNodes[dirIndex];
                            break;
                        } else {
                            break;
                        }
                    } else {
                        itNode = itNode->ChildNodes[dirIndex];
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    } else {
        node = centralNode;
        ImGuiDockNode *itNode = node;
        while (itNode->ParentNode) {
            ImGuiDockNode *upNode = itNode->ParentNode;
            if (upNode->SplitAxis == axis) {
                int itDirIndex = itNode == upNode->ChildNodes[0] ? 1 : 0;
                if (itDirIndex == dirIndex) {
                    if (!upNode->ChildNodes[dirIndex]->IsSplitNode()) {
                        dir = ImGuiDir_None;
                        node = upNode->ChildNodes[dirIndex];
                        break;
                    } else {
                        break;
                    }
                } else {
                    itNode = upNode;
                }
            } else {
                break;
            }
        }
    }

    DockContextQueueDock(GImGui, nullptr, node, window, dir, ratio, outer);
}

void MakeTabVisible(const char *name)
{
    ImGuiWindow *window = ImGui::FindWindowByName(name);
    if (window == NULL || window->DockNode == NULL || window->DockNode->TabBar == NULL)
        return;
    window->DockNode->TabBar->NextSelectedTabId = window->TabId;
}
}
