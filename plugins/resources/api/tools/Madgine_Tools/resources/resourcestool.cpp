#include "../resourcestoolslib.h"

#include "resourcestool.h"

#include "Generic/projections.h"

#include "Interfaces/process/processapi.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/resources/resourcebase.h"
#include "Madgine/resources/resourceloaderbase.h"
#include "Madgine/resources/resourcemanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/util/trace_imgui.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"
#include "resourceeditor.h"

UNIQUECOMPONENT(Engine::Tools::ResourcesTool);

METATABLE_BEGIN_BASE(Engine::Tools::ResourcesTool, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::ResourcesTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ResourcesTool, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ResourcesTool)

namespace Engine {
namespace Tools {

    ResourcesTool::ResourcesTool(ImRoot &root)
        : Tool<ResourcesTool>(root)
    {
    }

    Threading::Task<bool> ResourcesTool::init()
    {
        if (!co_await ToolBase::init())
            co_return false;

        co_await Resources::ResourceManager::getSingleton().state();

        refresh();

        co_return true;
    }

    void ResourcesTool::refresh()
    {
        mResources.clear();

        for (Resources::ResourceLoaderBase *loader : Resources::ResourceManager::getSingleton().mCollector | std::views::transform(projectionUniquePtrToPtr)) {
            for (Resources::ResourceBase *res : loader->resources()) {
                if (!res->path().empty()) {
                    mResources.push_back({ res, loader });
                }
            }
        }
    }

    std::string_view ResourcesTool::key() const
    {
        return "Resources";
    }

    void ResourcesTool::update()
    {
        ToolBase::update();
    }

    void ResourcesTool::render()
    {
        if (beginToolPanel("Resources", &mVisible, ImGuiDir_Down) && !ImGui::IsWindowAppearing()) {
            int count = mResources.size();

            ImGuiIO &io = ImGui::GetIO();

            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            float avail_width = ImGui::GetContentRegionAvail().x;

            // Layout: when not stretching: allow extending into right-most spacing.
            float layoutItemSpacing = 10.0f;
            float iconHitSpacing = 4.0f;
            if (false)
                avail_width += floorf(layoutItemSpacing * 0.5f);

            // Layout: calculate number of icon per line and number of lines
            ImVec2 layoutItemSize = ImVec2(floorf(mIconSize), floorf(mIconSize + 1.1f * ImGui::GetFontSize()));
            int layoutColumnCount = std::max((int)(avail_width / (layoutItemSize.x + layoutItemSpacing)), 1);
            int layoutLineCount = (count + layoutColumnCount - 1) / layoutColumnCount;

            // Layout: when stretching: allocate remaining space to more spacing. Round before division, so item_spacing may be non-integer.
            if (layoutColumnCount > 1)
                layoutItemSpacing = floorf(avail_width - layoutItemSize.x * layoutColumnCount) / layoutColumnCount;

            ImVec2 LayoutItemStep = ImVec2(layoutItemSize.x + layoutItemSpacing, layoutItemSize.y + layoutItemSpacing);
            float layoutSelectableSpacing = std::max(floorf(layoutItemSpacing) - iconHitSpacing, 0.0f);
            float layoutOuterPadding = floorf(layoutItemSpacing * 0.5f);

            // Calculate and store start position.
            ImVec2 start_pos = ImGui::GetCursorScreenPos();
            start_pos = ImVec2(start_pos.x + layoutOuterPadding, start_pos.y + layoutOuterPadding);
            ImGui::SetCursorScreenPos(start_pos);

            // Multi-select
            ImGuiMultiSelectFlags ms_flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;

            // - Enable box-select (in 2D mode, so that changing box-select rectangle X1/X2 boundaries will affect clipped items)
            ms_flags |= ImGuiMultiSelectFlags_BoxSelect2d;

            // - Enable keyboard wrapping on X axis
            // (FIXME-MULTISELECT: We haven't designed/exposed a general nav wrapping api yet, so this flag is provided as a courtesy to avoid doing:
            //    ImGui::NavMoveRequestTryWrapping(ImGui::GetCurrentWindow(), ImGuiNavMoveFlags_WrapX);
            // When we finish implementing a more general API for this, we will obsolete this flag in favor of the new system)
            ms_flags |= ImGuiMultiSelectFlags_NavWrapX;

            ImGuiMultiSelectIO *ms_io = ImGui::BeginMultiSelect(ms_flags, Selection.Size, count);

            // Use custom selection adapter: store ID in selection (recommended)
            Selection.UserData = this;
            Selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage *self_, int idx) { return static_cast<ImGuiID>(idx); };
            Selection.ApplyRequests(ms_io);

            const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (Selection.Size > 0)) || mDeleteRequested;
            const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop(ms_io, count) : -1;
            mDeleteRequested = false;

            // Push LayoutSelectableSpacing (which is LayoutItemSpacing minus hit-spacing, if we decide to have hit gaps between items)
            // Altering style ItemSpacing may seem unnecessary as we position every items using SetCursorScreenPos()...
            // But it is necessary for two reasons:
            // - Selectables uses it by default to visually fill the space between two items.
            // - The vertical spacing would be measured by Clipper to calculate line height if we didn't provide it explicitly (here we do).
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(layoutSelectableSpacing, layoutSelectableSpacing));

            // Rendering parameters
            const ImU32 icon_bg_color = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
            const bool display_label = (layoutItemSize.x >= ImGui::CalcTextSize("999").x);

            const int column_count = layoutColumnCount;
            ImGuiListClipper clipper;
            clipper.Begin(layoutLineCount, LayoutItemStep.y);
            if (item_curr_idx_to_focus != -1)
                clipper.IncludeItemByIndex(item_curr_idx_to_focus / column_count); // Ensure focused item line is not clipped.
            if (ms_io->RangeSrcItem != -1)
                clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem / column_count); // Ensure RangeSrc item line is not clipped.
            while (clipper.Step()) {
                for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; line_idx++) {
                    const int item_min_idx_for_current_line = line_idx * column_count;
                    const int item_max_idx_for_current_line = std::min((line_idx + 1) * column_count, count);
                    for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx) {
                        ResourceEntry &resource = mResources[item_idx];

                        ImGui::PushID(resource.mResource);

                        // Position item
                        ImVec2 pos = ImVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
                        ImGui::SetCursorScreenPos(pos);

                        ImGui::SetNextItemSelectionUserData(item_idx);
                        bool item_is_selected = Selection.Contains(static_cast<ImGuiID>(item_idx));
                        bool item_is_visible = ImGui::IsRectVisible(layoutItemSize);

                        ImGui::Selectable("", item_is_selected, ImGuiSelectableFlags_None, layoutItemSize);

                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            open(resource.mResource, resource.mLoader);
                        }

                        // Update our selection state immediately (without waiting for EndMultiSelect() requests)
                        // because we use this to alter the color of our text/icon.
                        if (ImGui::IsItemToggledSelection())
                            item_is_selected = !item_is_selected;

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == item_idx)
                            ImGui::SetKeyboardFocusHere(-1);

                        UndoStack stack;
                        TracedRoot traced { stack, ScopePtr { resource.mResource, resource.mLoader->resourceTypes().back() } };
                        ImGui::DraggableValueTypeSource<ScopePtr>("Resource", traced);

                        // Render icon (a real app would likely display an image/thumbnail here)
                        // Because we use ImGuiMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
                        if (item_is_visible) {
                            ImVec2 box_min(pos.x - 1, pos.y - 1);
                            ImVec2 box_max(box_min.x + layoutItemSize.x + 2, box_min.y + layoutItemSize.y + 2 - 1.1f * ImGui::GetFontSize()); // Dubious
                            draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color
                            if (!resource.mLoader->iconPath(resource.mResource).empty()) {
                                mRoot.DrawImage(resource.mLoader->iconPath(resource.mResource), { static_cast<int>(box_min.x), static_cast<int>(box_min.y) }, { static_cast<int>(box_max.x - box_min.x), static_cast<int>(box_max.y - box_min.y) }, 0.3f * mIconSize);
                            } else {
                                draw_list->AddText(box_min, ImColor { 255, 255, 255, 255 }, resource.mResource->extension().data());
                            }
                            if (display_label) {
                                draw_list->PushClipRect({ box_min.x, box_max.y }, { box_max.x, box_max.y + 1.1f * ImGui::GetFontSize() }, true);
                                ImU32 label_col = ImGui::GetColorU32(item_is_selected || true ? ImGuiCol_Text : ImGuiCol_TextDisabled);
                                draw_list->AddText(ImVec2(box_min.x, box_max.y), label_col, resource.mResource->name().data());
                                draw_list->PopClipRect();
                            }
                        }

                        ImGui::PopID();
                    }
                }
            }
            clipper.End();
            ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing

            // Context menu
            if (ImGui::BeginPopupContextWindow()) {
                if (Selection.Size == 1) {
                    void *p = nullptr;
                    ImGuiID id;
                    Selection.GetNextSelectedItem(&p, &id);
                    Resources::ResourceBase *res = mResources[id].mResource;
                    ImGui::Text("Source: " + res->plugin());
                    ImGui::Separator();
                }
                if (ImGui::MenuItem(IMGUI_ICON_X " Delete", "Del", false, Selection.Size > 0))
                    mDeleteRequested = true;
                ImGui::EndPopup();
            }

            ms_io = ImGui::EndMultiSelect();
            Selection.ApplyRequests(ms_io);
            if (want_delete)
                Selection.ApplyDeletionPostLoop(ms_io, mResources, item_curr_idx_to_focus);

            // Zooming with CTRL+Wheel
            if (ImGui::IsWindowAppearing())
                ZoomWheelAccum = 0.0f;
            if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f && ImGui::IsKeyDown(ImGuiMod_Ctrl) && !ImGui::IsAnyItemActive()) {
                ZoomWheelAccum += io.MouseWheel;
                if (fabsf(ZoomWheelAccum) >= 1.0f) {
                    // Calculate hovered item index from mouse location
                    // FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
                    const float hovered_item_nx = (io.MousePos.x - start_pos.x + layoutItemSpacing * 0.5f) / LayoutItemStep.x;
                    const float hovered_item_ny = (io.MousePos.y - start_pos.y + layoutItemSpacing * 0.5f) / LayoutItemStep.y;
                    const int hovered_item_idx = ((int)hovered_item_ny * layoutColumnCount) + (int)hovered_item_nx;
                    // ImGui::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

                    // Zoom
                    mIconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
                    mIconSize = std::clamp(mIconSize, 16.0f, 128.0f);
                    ZoomWheelAccum -= (int)ZoomWheelAccum;

                    // Layout: calculate number of icon per line and number of lines
                    ImVec2 layoutItemSize = ImVec2(floorf(mIconSize), floorf(mIconSize));
                    int layoutColumnCount = std::max((int)(avail_width / (layoutItemSize.x + layoutItemSpacing)), 1);

                    // Manipulate scroll to that we will land at the same Y location of currently hovered item.
                    // - Calculate next frame position of item under mouse
                    // - Set new scroll position to be used in next ImGui::BeginChild() call.
                    float hovered_item_rel_pos_y = ((float)(hovered_item_idx / layoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
                    hovered_item_rel_pos_y += ImGui::GetStyle().WindowPadding.y;
                    float mouse_local_y = io.MousePos.y - ImGui::GetWindowPos().y;
                    ImGui::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
                }
            }
        }
        ImGui::End();
    }

    void ResourcesTool::renderMenu()
    {
        ToolBase::renderMenu();

        if (ImGui::BeginMenu("Resources")) {
            if (ImGui::BeginMenu("New..."))
                ImGui::EndMenu();
            ImGui::Separator();
            ImGui::EndMenu();
        }
    }

    void ResourcesTool::registerEditor(Resources::ResourceLoaderBase *loader, ResourceEditor *editor)
    {
        mEditors[loader] = editor;
    }

    void ResourcesTool::open(Resources::ResourceBase *res, Resources::ResourceLoaderBase *loader)
    {
        auto it = mEditors.find(loader);
        if (it != mEditors.end()) {
            it->second->open(res);
        } else {
            Process::execute(res->path());
        }
    }

    bool ResourcesTool::lastFocusedEditor(ResourceEditor *editor)
    {
        if (ImGui::IsWindowFocused())
            mLastFocusedEditor = editor;

        return mLastFocusedEditor == editor;
    }

}
}