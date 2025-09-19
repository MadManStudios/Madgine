#include "../widgetstoolslib.h"

#include "widgeteditor.h"

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

#include "Madgine/widgets/widgetmanager.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Generic/coroutines/generator.h"

#include "Madgine_Tools/imguiicons.h"

#include "Madgine/widgets/widgetloader.h"

UNIQUECOMPONENT(Engine::Tools::WidgetEditor);

METATABLE_BEGIN_BASE(Engine::Tools::WidgetEditor, Engine::Tools::ToolBase)
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

        co_return co_await ResourceEditor::init(Widgets::WidgetLoader::getSingleton(), "Widget");
    }

    Threading::Task<void> WidgetEditor::finalize()
    {
        mSettings.clear();

        mFiles.clear();

        co_await ResourceEditor::finalize();
        co_return;
    }

    void WidgetEditor::render()
    {
        Widgets::WidgetBase *hoveredWidget = nullptr;
        renderHierarchy(&hoveredWidget);
        renderSelection(hoveredWidget);
    }

    void WidgetEditor::renderMenu()
    {
        ResourceEditor::renderMenu();
        if (mVisible) {

            if (ImGui::BeginMenu("WidgetEditor")) {

                for (Widgets::WidgetBase *w : mWidgetManager->widgets()) {
                    if (ImGui::MenuItem(w->key().c_str(), nullptr, w->mVisible)) {
                        mWidgetManager->swapCurrentRoot(w);
                    }
                }
                if (ImGui::Button("Create Layout")) {
                    mWidgetManager->createTopLevel();
                }

                ImGui::EndMenu();
            }
        }
    }

    Widgets::WidgetManager &WidgetEditor::manager()
    {
        return *mWidgetManager;
    }

    void WidgetEditor::update()
    {
        std::erase_if(mFiles, [](std::pair<Widgets::WidgetLoader::Resource *const, WidgetFile> &p) {
            return !p.second.render();
        });

        ResourceEditor::update();
    }

    std::string_view WidgetEditor::key() const
    {
        return "WidgetEditor";
    }

    void WidgetEditor::open(Resources::ResourceBase *res)
    {
        Widgets::WidgetLoader::Resource *widget = static_cast<Widgets::WidgetLoader::Resource *>(res);

        mFiles.try_emplace(widget, *this, widget->loadData());
    }

    void renderWidgetBorders(Widgets::WidgetBase *widget, Engine::Vector2i screenOffset, ImU32 color, ImDrawList *drawList)
    {
        ImGuiIO &io = ImGui::GetIO();

        Vector3 absoluteSize = widget->getAbsoluteSize();
        Vector2 absolutePos = widget->getAbsolutePosition() + Vector2 { screenOffset };

        Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

        drawList->AddRect(ImVec2 { bounds.topLeft() } / io.DisplayFramebufferScale, ImVec2 { bounds.bottomRight() } / io.DisplayFramebufferScale, color);
    }

    void WidgetEditor::renderSelection(Widgets::WidgetBase *hoveredWidget)
    {
        constexpr float borderSize = 10.0f;

        if (beginDefaultWindow(ImGuiDir_Right)) {

            ImDrawList *background = ImGui::GetBackgroundDrawList(ImGui::GetMainViewport());

            Rect2i screenSpace = mWidgetManager->getClientSpace();

            InterfacesVector pos = mWidgetManager->window().osWindow()->renderPos();
            Vector3i windowPos = Vector3i {
                pos.x, pos.y, 0
            };

            ImGuiIO &io = ImGui::GetIO();

            Vector2 mouse = ImGui::GetMousePos();
            Vector2 dragDistance = mouse - Vector2 { io.MouseClickedPos[0] };

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                screenSpace.mTopLeft += mWidgetManager->getScreenSpace().mTopLeft;

            bool acceptHover = (hoveredWidget != nullptr || !io.WantCaptureMouse);

            if (mSelected) {
                Widgets::WidgetBase *selectedWidget = mSelected->widget();

                Vector3 absoluteSize = selectedWidget->getAbsoluteSize();
                Vector2 absolutePos = selectedWidget->getAbsolutePosition() + Vector2 { screenSpace.mTopLeft };

                Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

                background->AddRect(ImVec2 { bounds.topLeft() } / io.DisplayFramebufferScale, ImVec2 { bounds.bottomRight() } / io.DisplayFramebufferScale, IM_COL32(255, 255, 255, 255));

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

                        background->AddRect(ImVec2 { bounds.topLeft() } / io.DisplayFramebufferScale, ImVec2 { bounds.bottomRight() } / io.DisplayFramebufferScale, IM_COL32(127, 127, 127, 255));
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

            // io.WantCaptureMouse = true;
        }
        ImGui::End();
    }

    bool WidgetEditor::drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget)
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
                for (const auto &[name, res] : Widgets::WidgetLoader::getSingleton()) {
                    if (ImGui::MenuItem(name.c_str())) {
                        Widgets::WidgetLoader::Handle desc = Widgets::WidgetLoader::load(name);
                        w->createChildByDescriptor(desc);
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

    void WidgetEditor::renderHierarchy(Widgets::WidgetBase **hoveredWidget)
    {
        if (ImGui::Begin("WidgetEditor - Hierarchy", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            Widgets::WidgetBase *root = mWidgetManager->currentRoot();
            if (root) {
                if (ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::BeginMenu(IMGUI_ICON_PLUS " New Widget")) {
                        for (const auto &[name, res] : Widgets::WidgetLoader::getSingleton()) {
                            if (ImGui::MenuItem(name.c_str())) {
                                Widgets::WidgetLoader::Handle desc = Widgets::WidgetLoader::load(name);
                                root->createChildByDescriptor(desc);                                
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
