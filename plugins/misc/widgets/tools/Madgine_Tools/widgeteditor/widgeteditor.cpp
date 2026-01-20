#include "../widgetstoolslib.h"

#include "widgeteditor.h"

#include "Generic/coroutines/generator.h"

#include "Meta/math/bounds.h"
#include "Meta/serialize/streams/serializestream.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/render/rendertarget.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetloader.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/behaviortool.h"
#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/imgui/clientimroot.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

UNIQUECOMPONENT(Engine::Tools::WidgetEditor);

METATABLE_BEGIN_BASE(Engine::Tools::WidgetEditor, Engine::Tools::ToolBase)
    MEMBER(mFiles)
METATABLE_END(Engine::Tools::WidgetEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::WidgetEditor, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::WidgetEditor)

namespace Engine {
namespace Tools {

    WidgetEditor::WidgetEditor(ImRoot &root)
        : Tool<WidgetEditor, ResourceEditor>(root)
    {
    }

    Threading::Task<bool> WidgetEditor::init()
    {
        mWidgetManager = &static_cast<const ClientImRoot &>(mRoot).window().getWindowComponent<Widgets::WidgetManager>();

        mInspector = &getTool<Inspector>();

        co_return co_await ResourceEditor::init(Widgets::WidgetLoader::getSingleton(), "Widget");
    }

    Threading::Task<void> WidgetEditor::finalize()
    {
        mFiles.clear();

        co_await ResourceEditor::finalize();
        co_return;
    }

    void WidgetEditor::renderMenu()
    {
        ResourceEditor::renderMenu();
        if (mVisible) {
        }
    }

    Widgets::WidgetManager &WidgetEditor::manager()
    {
        return *mWidgetManager;
    }

    void WidgetEditor::render()
    {
        std::erase_if(mFiles, [&, this](std::pair<Widgets::WidgetLoader::Resource *const, WidgetFile> &p) {
            return !p.second.render();
        });

        ResourceEditor::render();

        if (beginGame()) {

            handleManagerInteractions(*mWidgetManager, mWidgetManager->getClientSpace().mTopLeft);

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("WidgetEditor")) {

                    for (const Widgets::LayoutWidget &w : mWidgetManager->layoutWidgets()) {
                        ImGui::PushID(&w);
                        if (ImGui::MenuItem(w.mName.c_str(), nullptr, w.mWidget.isSet() && std::get<0>(*w.mWidget)->mVisible, w.mWidget.isSet())) {
                            if (std::get<0>(*w.mWidget)->mVisible)
                                mWidgetManager->closeLayout(w.mName);
                            else
                                mWidgetManager->openLayout(w.mName);
                        }
                        ImGui::PopID();
                    }
                    ImGui::Separator();
                    if (ImGui::BeginMenu("Manage")) {
                        for (Widgets::LayoutWidget &w : mWidgetManager->layoutWidgets()) {
                            ImGui::PushID(&w);
                            if (ImGui::BeginMenu((w.mName + "###Layout").c_str())) {
                                ImGui::PushID(&w);
                                ImGui::InputText("Name", &w.mName);
                                ImGui::EnumCombo("Type", &w.mType);
                                if (ImGui::BeginTable("LayoutWidgetMenuTable", 2, ImGuiTableFlags_SizingStretchProp)) {
                                    ScopePtr widgetTemplate { &w.mWidgetTemplate };
                                    if (mInspector->drawValue("Template", widgetTemplate, true).first) {
                                        w.mWidgetTemplate = scope_cast<Widgets::WidgetLoader::Resource>(widgetTemplate);
                                        if (w.mWidget.isSet()) {
                                            mWidgetManager->destroyTopLevel(std::get<0>(*w.mWidget));
                                            w.mWidget.reset();
                                        }
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::Checkbox("Default Visible", &w.mDefaultVisibility);
                                ImGui::PopID();
                                ImGui::EndMenu();
                            }
                            ImGui::PopID();
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Create Layout")) {
                            mWidgetManager->createLayout("Unnamed");
                        }
                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Panels")) {

                    ImGui::MenuItem("Widgets - Hierarchy", nullptr, &mGameHierarchyVisible);
                    ImGui::MenuItem("Widgets - Details", nullptr, &mGameDetailsVisible);

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            Widgets::WidgetBase *hovered = nullptr;
            renderHierarchy(&hovered);
            renderSelection();
        }
        ImGui::End();
    }

    std::string_view WidgetEditor::key() const
    {
        return "WidgetEditor";
    }

    void WidgetEditor::open(Resources::ResourceBase *res)
    {
        Widgets::WidgetLoader::Resource *widget = static_cast<Widgets::WidgetLoader::Resource *>(res);

        mFiles.try_emplace(widget, *this, widget);
    }

    void WidgetEditor::renderWidgetBorders(Widgets::WidgetBase *widget, Engine::Vector2i screenOffset, ImU32 color)
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        ImGuiIO &io = ImGui::GetIO();

        Vector3 absoluteSize = widget->getAbsoluteSize();
        Vector2 absolutePos = widget->getAbsolutePosition() + Vector2 { screenOffset };

        Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

        drawList->AddRect(ImVec2 { bounds.topLeft() } / io.DisplayFramebufferScale, ImVec2 { bounds.bottomRight() } / io.DisplayFramebufferScale, color);
    }

    void WidgetEditor::renderHierarchy(Widgets::WidgetBase **hoveredWidget)
    {
        if (mGameHierarchyVisible) {
            if (beginSubPanel("Widgets - Hierarchy", &mGameHierarchyVisible, ImGuiDir_Left)) {

                for (Widgets::LayoutWidget &layoutWidget : manager().layoutWidgets()) {
                    Widgets::WidgetBase *widget = layoutWidget.mWidget.isSet() ? std::get<0>(*layoutWidget.mWidget) : nullptr;
                    if (widget && widget->mVisible) {
                        if (ImGui::TreeNode(widget->mName.c_str())) {
                            drawWidget(widget, hoveredWidget);
                            ImGui::TreePop();
                        }
                    }
                }

                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0)) {
                    if (hoveredWidget && *hoveredWidget)
                        mSelected = *hoveredWidget;
                    else
                        mSelected = nullptr;
                }
            }
            ImGui::End();
        }
    }

    void WidgetEditor::renderSelection()
    {
        if (mGameDetailsVisible) {
            if (beginSubPanel("Widgets - Details", &mGameDetailsVisible, ImGuiDir_Right)) {
                if (mSelected) {
                    mInspector->getTool<DebuggerView>().renderLifetime(mSelected->lifetime());

                    bool showParameters = false;
                    if (ImGui::BeginPopupCompoundContextWindow()) {

                        if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Behavior")) {
                            if (Behavior::BehaviorHandle behavior = BehaviorSelector()) {
                                mPendingBehavior.mHandle = std::move(behavior);
                                mPendingBehavior.mParameters = mPendingBehavior.mHandle.createParameters();
                                showParameters = true;
                            }
                            ImGui::EndMenu();
                        }

                        ImGui::EndPopup();
                    }

                    if (showParameters)
                        ImGui::OpenPopup("BehaviorParameters");

                    if (ImGui::BeginPopup("BehaviorParameters")) {
                        if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_SizingStretchProp)) {
                            mInspector->drawMembers(&mPendingBehavior.mParameters);
                            ImGui::EndTable();
                        }
                        if (ImGui::Button("Cancel")) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Create Behavior")) {
                            mSelected->addBehavior(mPendingBehavior.mHandle.create(mPendingBehavior.mParameters));
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }
            }
            ImGui::End();
        }
    }

    Widgets::WidgetBase *WidgetEditor::handleManagerInteractions(Widgets::WidgetManager &manager, const ImVec2 &pos)
    {
        Widgets::WidgetBase *hoveredWidget = manager.hoveredWidget();

        WidgetSettings *hoveredSettings = nullptr;
        if (hoveredWidget) {
            Rect2i screenSpace = manager.getClientSpace();
            screenSpace.mTopLeft = { static_cast<int>(pos.x), static_cast<int>(pos.y) };

            ImGuiIO &io = ImGui::GetIO();

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                screenSpace.mTopLeft += manager.getScreenSpace().mTopLeft;

            renderWidgetBorders(hoveredWidget, screenSpace.mTopLeft, IM_COL32(127, 127, 127, 255));
        }

        return hoveredWidget;
    }

    void WidgetEditor::drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        if (w->children().empty())
            flags |= ImGuiTreeNodeFlags_Leaf;
        if (mSelected == w)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::TreeNodeEx(w->mName.c_str(), flags);
        if (hoveredWidget && !*hoveredWidget) {
            if (ImGui::IsItemHovered()) {
                *hoveredWidget = w;
            }
        }

        if (open) {
            for (Widgets::WidgetBase *child : w->children()) {
                drawWidget(child, hoveredWidget);
            }
            ImGui::TreePop();
        }
    }

}
}
