#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Generic/coroutines/generator.h"
#include "Generic/enum.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/filesystem/path.h"

#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/scopeptr.h"
#include "Meta/keyvalue/valuetype.h"
#include "Meta/math/matrix3.h"

#include "Modules/threading/threadlocal.h"
#include "Modules/threading/workgroupstorage.h"

#include "../imgui/imgui_internal.h"
#include "imguiaddons.h"

// Engine::Threading::WorkgroupLocal<ImGuiContext *>
THREADLOCAL(ImGuiContext *)
sContext;

ImGuiContext *&getImGuiContext()
{
    return sContext;
}

namespace ImGui {

struct GroupPanelData {
    ImRect mRect;
    ImU32 mColor;
};

static Engine::Threading::WorkgroupLocal<ImVector<GroupPanelData>> sGroupPanelLabelStack;

static Engine::Threading::WorkgroupLocal<FilesystemPickerOptions> sFilesystemPickerOptions;

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

    bool value_changed = InputTextEx(label, NULL, s->data(), s->size() + 1, bb.GetSize(), flags | ImGuiInputTextFlags_TempInput | ImGuiInputTextFlags_CallbackResize, &InputTextCallback<std::string>, &cb);
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
    bool b = Selectable("##Selectable", flags & ImGuiTreeNodeFlags_Selected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap);
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
    // ImGui::Dummy({ 0, 0 });
    // ImGui::SameLine(avail - size);
    // window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + (avail - size) + window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
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

bool LED(const char *label, bool on, const ImVec2 &size)
{
    ImGui::BeginGroup();
    ImGui::PushID(label);

    // Get current window and draw list
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
        ImGui::PopID();
        ImGui::EndGroup();
        return false;
    }
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    // Calculate position for the LED
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(p.x + size.x * 0.5f, p.y + size.y * 0.5f);
    float radius = size.x * 0.2f;

    // Define colors: a darker grey for 'off', and a vibrant color for 'on'
    ImU32 color_on = ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Bright green
    ImU32 color_off = ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Grey

    // Draw the LED (circle)
    if (on) {
        // Optional: Add a subtle background glow for "on" state
        // This creates an outer, slightly transparent circle
        ImU32 glow_color = ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
        draw_list->AddCircleFilled(center, radius + 2.0f, glow_color, 16);
        draw_list->AddCircleFilled(center, radius, color_on, 16);
    } else {
        draw_list->AddCircleFilled(center, radius, color_off, 16);
    }

    // Handle interaction (if needed, e.g., make it a clickable toggle)
    // Here we just advance the cursor position so other widgets can follow
    bool pressed = ImGui::InvisibleButton("##LED_button", size);

    // Add label text if provided (after the LED, on the same line)
    if (label && label[0] != '#') // Check if it's not a hidden label (starting with ##)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (size.y - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::Text("%s", label);
    }

    ImGui::PopID();
    ImGui::EndGroup();

    return pressed;
}

bool LED(const char *label, bool *on, const ImVec2 &size)
{
    bool pressed = LED(label, *on, size);
    if (pressed)
        *on = !*on;
    return pressed;
}

bool BeginFilesystemPicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, const Engine::Filesystem::Path &base)
{
    bool changed = false;

    if (!base.empty()) {
        if (!path.isRelative(base))
            path = base.absolute();
        if (!selection.isRelative(base))
            selection = base.absolute();
    } else {
        if (path.empty())
            path = Engine::Filesystem::Path { "." }.absolute();
        if (selection.empty())
            selection = Engine::Filesystem::Path { "." }.absolute();
    }

    ImGui::BeginDisabled(path == base);
    if (ImGui::Button("Up")) {
        selection = path;
        path = path / "..";
        changed = true;
    }
    ImGui::EndDisabled();

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
    bool changed = BeginFilesystemPicker(path, selection, options.mBase);

    ImVec2 size = ImGui::GetContentRegionAvail();
    size.x -= 4.0f;
    size.y -= 4.0f;

    if (ImGui::BeginTable("CurrentFolder", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable, size)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        // ImGui::TableSetupColumn("Total time");
        // ImGui::TableSetupColumn("Rel. Time (parent)");
        // ImGui::TableSetupColumn("Rel. Time (total)");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (Engine::Filesystem::FileQueryResult result : Engine::Filesystem::listDirs(path)) {

            ImGui::TableNextRow();

            ImGui::TableNextColumn();

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
        ImGui::EndTable();
    }

    return changed;
}

FilesystemPickerOptions *GetFilesystemPickerOptions()
{
    return &sFilesystemPickerOptions;
}

bool FilePicker(Engine::Filesystem::Path &path, Engine::Filesystem::Path &selection, bool *itemDoubleClicked, const FilesystemPickerOptions &options)
{
    bool changed = BeginFilesystemPicker(path, selection, options.mBase);

    bool selectedIsFile = false;
    bool selectedIsDir = false;

    ImVec2 size = ImGui::GetContentRegionAvail();
    size.x -= 4.0f;
    size.y -= 4.0f;

    ImGuiContext &g = *GImGui;

    ImGuiStyle &style = g.Style;

    Engine::Filesystem::Path file = selection.relative(path);
    if (file == ".")
        file.clear();

    float elementHeight = CalcTextSize(file.c_str(), NULL, true).y + style.FramePadding.y * 2.0f;
    size.y -= elementHeight; // Filename
    if (!options.mExtensions.empty()) {
        size.y -= style.ItemSpacing.y;
        size.y -= elementHeight; // Extension
    }

    if (ImGui::BeginTable("CurrentFolder", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable, size)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        // ImGui::TableSetupColumn("Total time");
        // ImGui::TableSetupColumn("Rel. Time (parent)");
        // ImGui::TableSetupColumn("Rel. Time (total)");
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
                    if (itemDoubleClicked)
                        *itemDoubleClicked = true;
                }
            }
        }

        ImGui::EndTable();
    }

    std::string extension { file.extension() };
    if (extension.empty() && !options.mExtensions.empty()) {
        extension = options.mExtensions.front();
        file = file.str() + extension;
    }

    std::string baseName { file.stem() };

    if (InputText("Filename:", &baseName)) {
        Engine::Filesystem::Path p = baseName + extension;
        if (!p.empty() && p.isRelative())
            selection = path / p;

        changed = true;
    }

    if (!options.mExtensions.empty()) {
        if (BeginCombo("Extension", extension.c_str())) {
            for (const std::string &ext : options.mExtensions) {
                if (Selectable(ext.c_str(), ext == extension)) {
                    extension = ext;
                    Engine::Filesystem::Path p = baseName + extension;
                    if (!p.empty() && p.isRelative())
                        selection = path / p;

                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
    }

    return changed;
}

InteractiveViewResultFlags InteractiveView(ImGuiID id)
{
    ImGuiContext &g = *GImGui;

    ImGui::ItemAdd(g.LastItemData.Rect, id);

    return InteractiveView();
}

InteractiveViewResultFlags InteractiveView()
{
    ImGuiContext &g = *GImGui;

    InteractiveViewResultFlags result = 0;

    int saved_mouse_button = g.ActiveIdMouseButton;

    ImGuiID id = g.LastItemData.ID;

    bool hovered = false;
    bool held = false;
    bool pressed = ImGui::ButtonBehavior(g.LastItemData.Rect, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_PressedOnClickRelease);

    bool active = IsItemActive();

    bool isDragging = false;

    if (active) {
        result |= InteractiveViewResultFlags_Active;
        result |= g.ActiveIdMouseButton + 1;
        isDragging = IsMouseDragPastThreshold(g.ActiveIdMouseButton, g.IO.MouseDragThreshold * 0.5f);
        if (isDragging && !WasMouseDragPastThreshold(g.ActiveIdMouseButton, g.IO.MouseDragThreshold * 0.5f)) {
            result |= InteractiveViewResultFlags_DragStarted;
        }
    } else if (IsItemDeactivated()) {
        result |= saved_mouse_button + 1;
        isDragging = IsMouseDragPastThreshold(saved_mouse_button, g.IO.MouseDragThreshold * 0.5f);
        if (isDragging) {
            result |= InteractiveViewResultFlags_DragStopped;
        }
    }

    if (isDragging)
        result |= InteractiveViewResultFlags_Dragging;
    if (pressed && !isDragging)
        result |= InteractiveViewResultFlags_Pressed;
    if (IsItemHovered())
        result |= InteractiveViewResultFlags_Hovered;

    return result;
}

bool BeginPopupContextInteractiveView(ImGuiPopupFlags flags)
{
    ImGuiContext &g = *GImGui;
    ImGuiWindow *window = g.CurrentWindow;
    if (window->SkipItems)
        return false;
    ImGuiID id = g.LastItemData.ID; // If user hasn't passed an ID, we can use the LastItem ID. Using LastItem ID as a Popup ID won't conflict!
    IM_ASSERT(id != 0); // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)

    bool open = false;
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(flags);
    if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && !IsMouseDragPastThreshold(mouse_button, g.IO.MouseDragThreshold * 0.5f))
        open = true;
    if (g.NavOpenContextMenuItemId == id && (IsItemFocused() || id == g.CurrentWindow->MoveId))
        open = true;
    if (open)
        OpenPopupEx(id, flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

void ImGui::BeginGroupPanel(const char *name, const ImVec2 &size, ImU32 backgroundColor)
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

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 0.5f * frameHeight);
    ImGui::BeginGroup();

    // ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->ContentRegionRect.Min.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Min.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Min.x += frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->ContentsRegionRect.Min.x += frameHeight * 0.5f;
#endif
    ImGui::GetCurrentWindow()->Size.x -= frameHeight;

    auto itemWidth = ImGui::CalcItemWidth();
    ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

    if (strlen(name) > 0)
        sGroupPanelLabelStack->push_back({ { labelMin, labelMax }, backgroundColor });
    else
        sGroupPanelLabelStack->push_back({ {}, backgroundColor });
}

void ImGui::EndGroupPanel()
{
    ImGui::PopItemWidth();

    auto itemSpacing = ImGui::GetStyle().ItemSpacing;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    auto frameHeight = ImGui::GetFrameHeight();

    ImGui::EndGroup();

    auto data = sGroupPanelLabelStack->back();
    sGroupPanelLabelStack->pop_back();

    // if (data.mColor > 0)
    //     ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), data.mColor, 4.0f);

    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
    ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

    ImGui::EndGroup();

    auto itemMin = ImGui::GetItemRectMin();
    auto itemMax = ImGui::GetItemRectMax();

    auto labelRect = data.mRect;

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

    if (data.mColor > 0)
        ImGui::GetWindowDrawList()->AddRectFilled(frameRect.Min, frameRect.Max, data.mColor, 4.0f);

    ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
    ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->ContentRegionRect.Min.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->WorkRect.Min.x -= frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->InnerRect.Min.x -= frameHeight * 0.5f;
#else
    ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
    ImGui::GetCurrentWindow()->ContentsRegionRect.Min.x -= frameHeight * 0.5f;
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
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
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

bool Spinner(const char *label, float radius, int thickness, const ImU32 &color)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

    const ImRect bb(pos, pos + size);
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    DrawSpinner(bb.Min, bb.Max, radius, thickness, color);

    return true;
}

void DrawSpinner(const ImVec2 &min, const ImVec2 &max, float radius, int thickness, const ImU32 &color)
{
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext &g = *GImGui;

    // Render
    window->DrawList->PathClear();

    int num_segments = 30;
    int start = abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

    const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

    const ImVec2 center = ImRect { min, max }.GetCenter();

    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(center.x + ImCos(a + g.Time * 8) * radius,
            center.y + ImSin(a + g.Time * 8) * radius));
    }

    window->DrawList->PathStroke(color, false, thickness);
}

bool BeginStatus()
{
    if (BeginViewportSideBar("##MainStatusBar", GetMainViewport(), ImGuiDir_Down, GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
        if (BeginMenuBar()) {
            return true;
        } else {
            End();
        }
    }
    return false;
}

void EndStatus()
{
    Separator();
    EndMenuBar();
    End();
}

bool BeginToolBar(const char *name)
{
    ImGuiStyle &style = ImGui::GetStyle();
    const float height = style.FramePadding[ImGuiAxis_Y] * 2.0f + style.WindowPadding[ImGuiAxis_Y] * 2.0f + ImGui::GetFontSize();

    if (ImGui::BeginChild("Toolbar", { 0.0f, height }, ImGuiChildFlags_NavFlattened | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar)) {

        ImGui::BeginHorizontal(name);

        return true;
    } else {
        ImGui::EndChild();
        return false;
    }
}

void EndToolBar()
{
    ImGui::EndHorizontal();
    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::EndChild();
}

bool WasMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext &g = *GImGui;

    ImGuiIO &io = g.IO;

    if (!io.MouseDown[button])
        return false;

    float MouseDragMaxDistanceSqr;

    ImVec2 delta_from_click_pos = io.MousePosPrev - io.MouseClickedPos[button];
    MouseDragMaxDistanceSqr = ImLengthSqr(delta_from_click_pos);

    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    return MouseDragMaxDistanceSqr >= lock_threshold * lock_threshold;
}

}
