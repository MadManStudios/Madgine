#include "../widgetstoolslib.h"

#include "guieditor.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "imgui/imgui.h"

#include "imgui/imguiaddons.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/math/bounds.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/widgets/widget.h"
#include "Madgine/window/mainwindow.h"

#include "Interfaces/window/windowapi.h"

#include "Meta/serialize/streams/serializestream.h"

#include "Madgine/widgets/widgetclass.h"

#include "Madgine/widgets/widgetmanager.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Generic/coroutines/generator.h"

#include "Madgine_Tools/imguiicons.h"

UNIQUECOMPONENT(Engine::Tools::GuiEditor);

METATABLE_BEGIN_BASE(Engine::Tools::GuiEditor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::GuiEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::GuiEditor, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::GuiEditor)

namespace Engine {
namespace Tools {

    GuiEditor::GuiEditor(ImRoot &root)
        : Tool<GuiEditor>(root)
    {
    }

    Threading::Task<bool> GuiEditor::init()
    {
        mWidgetManager = &static_cast<const ClientImRoot &>(mRoot).window().getWindowComponent<Widgets::WidgetManager>();

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> GuiEditor::finalize()
    {
        mSettings.clear();

        co_await ToolBase::finalize();
        co_return;
    }

    void GuiEditor::render()
    {
        Widgets::WidgetBase *hoveredWidget = nullptr;
        renderHierarchy(&hoveredWidget);
        renderSelection(hoveredWidget);
    }

    void GuiEditor::renderMenu()
    {
        ToolBase::renderMenu();
        if (mVisible) {

            if (ImGui::BeginMenu("GuiEditor")) {

                for (Widgets::WidgetBase *w : mWidgetManager->widgets()) {
                    if (ImGui::MenuItem(w->key().c_str(), nullptr, w->mVisible)) {
                        mWidgetManager->swapCurrentRoot(w);
                    }
                }
                if (ImGui::Button("Create Layout")) {
                    mWidgetManager->createTopLevel<>();
                }

                ImGui::EndMenu();
            }
        }
    }

    void GuiEditor::update()
    {
        ToolBase::update();
    }

    std::string_view GuiEditor::key() const
    {
        return "GuiEditor";
    }

    void renderWidgetBorders(Widgets::WidgetBase *widget, Engine::Vector2i screenOffset, ImU32 color, ImDrawList *drawList)
    {
        ImGuiIO &io = ImGui::GetIO();
        
        Vector3 absoluteSize = widget->getAbsoluteSize();
        Vector2 absolutePos = widget->getAbsolutePosition() + Vector2 { screenOffset };

        Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);
              

        drawList->AddRect(bounds.topLeft() / io.DisplayFramebufferScale, bounds.bottomRight() / io.DisplayFramebufferScale, color);
    }

    void GuiEditor::renderSelection(Widgets::WidgetBase *hoveredWidget)
    {
        constexpr float borderSize = 10.0f;

        if (ImGui::Begin("GuiEditor", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Right, 0.2f, false, ImGuiCond_FirstUseEver);

            ImDrawList *background = ImGui::GetBackgroundDrawList(ImGui::GetMainViewport());

            Rect2i screenSpace = mWidgetManager->getClientSpace();

            InterfacesVector pos = mWidgetManager->window().osWindow()->renderPos();
            Vector3i windowPos = Vector3i {
                pos.x, pos.y, 0
            };

            ImGuiIO &io = ImGui::GetIO();

            Vector2 mouse = ImGui::GetMousePos();
            Vector2 dragDistance = mouse - io.MouseClickedPos[0];

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                screenSpace.mTopLeft += mWidgetManager->getScreenSpace().mTopLeft;

            bool acceptHover = (hoveredWidget != nullptr || !io.WantCaptureMouse);

            if (mSelected) {
                Widgets::WidgetBase *selectedWidget = mSelected->widget();

                Vector3 absoluteSize = selectedWidget->getAbsoluteSize();
                Vector2 absolutePos = selectedWidget->getAbsolutePosition() + Vector2 { screenSpace.mTopLeft };

                Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

                background->AddRect(bounds.topLeft() / io.DisplayFramebufferScale, bounds.bottomRight() / io.DisplayFramebufferScale, IM_COL32(255, 255, 255, 255));                

                if (!io.WantCaptureMouse) {

                    bool rightBorder = false, leftBorder = false, topBorder = false, bottomBorder = false;

                    bool hoveredWithBorder = selectedWidget->containsPoint(mouse, screenSpace, borderSize);

                    if (!mDragging && hoveredWithBorder) {

                        leftBorder = abs(mouse.x - bounds.left()) < borderSize;
                        rightBorder = abs(mouse.x - bounds.right()) < borderSize;
                        topBorder = abs(mouse.y - bounds.top()) < borderSize;
                        bottomBorder = abs(mouse.y - bounds.bottom()) < borderSize;

                        if (mSelected->aspectRatio()) {
                            if (topBorder || leftBorder) {
                                leftBorder = !rightBorder;
                                topBorder = !bottomBorder;
                            }
                            if (bottomBorder || rightBorder) {
                                rightBorder = !leftBorder;
                                bottomBorder = !topBorder;
                            }
                        }

                        acceptHover &= (!rightBorder && !leftBorder && !topBorder && !bottomBorder);

                        if (io.MouseClicked[0]) {
                            mMouseDown = true;
                            mDraggingLeft = leftBorder;
                            mDraggingRight = rightBorder;
                            mDraggingTop = topBorder;
                            mDraggingBottom = bottomBorder;
                        }
                    }

                    bool left = leftBorder || mDraggingLeft;
                    bool right = rightBorder || mDraggingRight;
                    bool top = topBorder || mDraggingTop;
                    bool bottom = bottomBorder || mDraggingBottom;
                    if (left || right) {
                        if (top || bottom) {
                            if (top == left) {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                            } else {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                            }
                        } else {
                            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                        }
                    } else {
                        if (top || bottom) {
                            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                        } else {
                            if (hoveredWithBorder && selectedWidget == mWidgetManager->hoveredWidget())
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                        }
                    }

                    if (mMouseDown && dragDistance.length() >= io.MouseDragThreshold && !mDragging) {
                        mSelected->saveGeometry();
                        mDragging = true;
                    }
                }
            }

            Widgets::WidgetBase *pointerEventTargetWidget = mWidgetManager->pointerEventTargetWidget();
            if (pointerEventTargetWidget)
                renderWidgetBorders(pointerEventTargetWidget, screenSpace.mTopLeft, IM_COL32(127, 100, 10, 255), background);

            Widgets::WidgetBase *focusedWidget = mWidgetManager->focusedWidget();
            if (focusedWidget)
                renderWidgetBorders(focusedWidget, screenSpace.mTopLeft, IM_COL32(255, 200, 10, 255), background);

            if (!hoveredWidget)
                hoveredWidget = mWidgetManager->hoveredWidget();

            if (acceptHover) {
                WidgetSettings *hoveredSettings = nullptr;
                if (hoveredWidget) {
                    hoveredSettings = &mSettings.try_emplace(hoveredWidget, hoveredWidget, getTool<Inspector>()).first->second;

                    if (!mDragging && hoveredSettings != mSelected) {
                        Vector3 size = hoveredWidget->getAbsoluteSize();
                        Vector2 pos = hoveredWidget->getAbsolutePosition() + Vector2 { screenSpace.mTopLeft };

                        Bounds bounds(pos.x, pos.y + size.y, pos.x + size.x, pos.y);

                        background->AddRect(bounds.topLeft() / io.DisplayFramebufferScale, bounds.bottomRight() / io.DisplayFramebufferScale, IM_COL32(127, 127, 127, 255));
                    }
                }
                if (io.MouseReleased[0] && !mDragging) {
                    mSelected = hoveredSettings;
                }
            }

            if (mSelected) {

                enum ResizeMode {
                    RELATIVE,
                    ABSOLUTE
                };

                ResizeMode resizeMode = RELATIVE;
                if (io.KeyShift) {
                    resizeMode = ABSOLUTE;
                }

                if (mDragging) {

                    auto [pos, size] = mSelected->savedGeometry();

                    Vector3 parentSize = mSelected->widget()->getParent() ? mSelected->widget()->getParent()->getAbsoluteSize() : Vector3 { Vector2 { screenSpace.mSize }, 1.0f };

                    Vector2 relDragDistance = dragDistance / parentSize.xy();

                    Matrix3 dragDistanceSize;

                    switch (resizeMode) {
                    case RELATIVE:
                        dragDistanceSize = Matrix3 {
                            relDragDistance.x, 0, 0,
                            0, relDragDistance.y, 0,
                            0, 0, 0
                        };
                        break;
                    case ABSOLUTE:
                        dragDistanceSize = Matrix3 {
                            0, 0, dragDistance.x / parentSize.z,
                            0, 0, dragDistance.y / parentSize.z,
                            0, 0, 0
                        };
                        break;
                    }

                    if (!mDraggingLeft && !mDraggingRight && !mDraggingTop && !mDraggingBottom) {
                        pos += dragDistanceSize;
                    } else {
                        Matrix3 dragDistancePos { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                        if (!mDraggingLeft && !mDraggingRight) {
                            dragDistanceSize[0][0] = 0.0f;
                            dragDistanceSize[0][2] = 0.0f;
                            dragDistancePos[0][0] = 0.0f;
                            dragDistancePos[0][2] = 0.0f;
                        } else if (mDraggingLeft) {
                            dragDistancePos[0][0] = dragDistanceSize[0][0];
                            dragDistancePos[0][2] = dragDistanceSize[0][2];
                            dragDistanceSize[0][0] *= -1.0f;
                            dragDistanceSize[0][2] *= -1.0f;
                        }
                        if (!mDraggingTop && !mDraggingBottom) {
                            dragDistanceSize[1][1] = 0.0f;
                            dragDistanceSize[1][2] = 0.0f;
                            dragDistancePos[1][1] = 0.0f;
                            dragDistancePos[1][2] = 0.0f;
                        } else if (mDraggingTop) {
                            if (mSelected->aspectRatio()) {
                                dragDistancePos[1][0] = -dragDistanceSize[0][0];
                                dragDistancePos[1][1] = 0.0f;
                                dragDistancePos[1][2] = -dragDistanceSize[0][2];
                            } else {
                                dragDistancePos[1][1] = dragDistanceSize[1][1];
                                dragDistancePos[1][2] = dragDistanceSize[1][2];
                            }
                            dragDistanceSize[1][1] *= -1.0f;
                            dragDistanceSize[1][2] *= -1.0f;
                        }

                        pos += dragDistancePos;
                        size += dragDistanceSize;
                    }

                    mSelected->setSize(size);
                    mSelected->setPos(pos);

                    if (io.MouseReleased[0]) {
                        mSelected->applyGeometry();
                        mDragging = false;
                    }
                }

                if (io.MouseReleased[0]) {
                    mMouseDown = false;
                    mDraggingLeft = false;
                    mDraggingRight = false;
                    mDraggingTop = false;
                    mDraggingBottom = false;
                }
            }

            if (mSelected) {
                mSelected->render();
            }

            //io.WantCaptureMouse = true;
        }
        ImGui::End();
    }

    bool GuiEditor::drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        if (w->children().empty())
            flags |= ImGuiTreeNodeFlags_Leaf;
        if (mSelected && mSelected->widget() == w)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::EditableTreeNode(w, &w->mName, flags);

        bool aborted = false;

        if (ImGui::BeginPopupCompoundContextItem()) {
            if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Child Widget")) {
                for (Widgets::WidgetClass c : Widgets::WidgetClass::values()) {
                    if (ImGui::MenuItem(std::string { c.toString() }.c_str())) {
                        w->createChild(c);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(IMGUI_ICON_X " Delete Widget", "del")) {
                w->destroy();
                aborted = true;
            }
            ImGui::EndPopup();
        }

        if (!aborted) {
            if (hoveredWidget && !*hoveredWidget) {
                if (ImGui::IsItemHovered()) {
                    *hoveredWidget = w;
                }
            }

            ImGui::DraggableValueTypeSource(w->mName, w);
            if (ImGui::BeginDragDropTarget()) {
                Widgets::WidgetBase *newChild = nullptr;
                if (ImGui::AcceptDraggableValueType(newChild, nullptr, [](const Widgets::WidgetBase *child) { return child->getParent(); })) {
                    newChild->setParent(w);
                    aborted = true;
                }
                ImGui::EndDragDropTarget();
            }
        }

        if (open) {
            if (!aborted) {
                for (Widgets::WidgetBase *child : w->children()) {
                    if (!drawWidget(child, hoveredWidget)) {
                        break;
                    }
                }
            }

            ImGui::TreePop();
        }

        return !aborted;
    }

    void GuiEditor::renderHierarchy(Widgets::WidgetBase **hoveredWidget)
    {
        if (ImGui::Begin("GuiEditor - Hierarchy", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            Widgets::WidgetBase *root = mWidgetManager->currentRoot();
            if (root) {
                if (ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::BeginMenu(IMGUI_ICON_PLUS " New Widget")) {
                        for (Widgets::WidgetClass c : Widgets::WidgetClass::values()) {
                            if (ImGui::MenuItem(std::string { c.toString() }.c_str())) {
                                root->createChild(c);
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                drawWidget(root, hoveredWidget);

                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
                    if (hoveredWidget && *hoveredWidget)
                        mSelected = &mSettings.try_emplace(*hoveredWidget, *hoveredWidget, getTool<Inspector>()).first->second;
                    else
                        mSelected = nullptr;
                }
            } else {
                ImGui::Text("Please select a root window under 'Layout' in the menu bar.");
            }
        }
        ImGui::End();
    }

}
}
