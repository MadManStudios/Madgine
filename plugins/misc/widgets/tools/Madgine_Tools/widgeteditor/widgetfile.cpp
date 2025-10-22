#include "../widgetstoolslib.h"

#include "widgeteditor.h"
#include "widgetfile.h"

#include "Madgine_Tools/imgui/clientimroot.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/render/rendercontext.h"

#include "Madgine/widgets/widgetmanager.h"

#include "Madgine/widgets/util/widgetsrenderdata.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine/widgets/widget.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/serialize/formats.h"

#include "Meta/serialize/container/container_operations.h"

#include "Madgine_Tools/imguiicons.h"
#include "Meta/math/bounds.h"
#include "imgui/imguiaddons.h"

#include "Interfaces/window/windowapi.h"

#include "Madgine_Tools/inspector/inspector.h"

namespace Engine {
namespace Tools {

    WidgetFile::WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Resource *resource)
        : mEditor(editor)
        , mPath(resource ? resource->path() : "")
        , mRenderTarget(editor.manager().window().getRenderer()->createRenderTexture({ 1, 1 }, { .mName { resource ? resource->name() : "<Unnamed>" }, .mFormat = Render::FORMAT_RGBA8_SRGB }))
        , mWidgetManager(editor.manager())
    {
        static_cast<ClientImRoot &>(editor.root()).addRenderTarget(mRenderTarget.get());
        mRenderTarget->addRenderPass(&mWidgetManager);

        mTopLevel = mWidgetManager.createTopLevel();

        if (resource) {
            Serialize::SerializeManager serializeMgr { "CompoundWidget" };
            Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), serializeMgr.wrapStream(resource->readAsStream(), true) };

            Serialize::StreamResult result = Serialize::read({ stream, CallerHierarchy { &mWidgetManager } }, *mTopLevel, "Widget");
            if (result.mState != Serialize::StreamState::OK) {
                LOG_ERROR(result);
                throw 0;
            }
        }

        mWidgetManager.swapCurrentRoot(mTopLevel);
    }

    WidgetFile::~WidgetFile()
    {
        mWidgetManager.clear();

        mRenderTarget->removeRenderPass(&mWidgetManager);
        static_cast<ClientImRoot &>(mEditor.root()).removeRenderTarget(mRenderTarget.get());
    }

    void WidgetFile::save(const Filesystem::Path &path)
    {
        Filesystem::FileManager mgr { "Widget" };

        Serialize::FormattedSerializeStream stream = mgr.openWrite(path, Serialize::Formats::xml);

        Serialize::write(stream, *mTopLevel, "Widget");

        mPath = path;
        mIsDirty = false;
    }

    void WidgetFile::renderSelection(const ImVec2 &pos, Widgets::WidgetBase *hoveredWidget)
    {
        if (mEditor.mWidgetDetailsVisible) {
            if (mEditor.beginSubPanel("Details", &mEditor.mWidgetDetailsVisible, ImGuiDir_Right)) {

                if (mSelected) {
                    mSelected->render();
                }

                // io.WantCaptureMouse = true;
            }
            ImGui::End();
        }
    }

    bool WidgetFile::drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget)
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

    void WidgetFile::renderHierarchy(Widgets::WidgetBase **hoveredWidget)
    {
        if (mEditor.mHierarchyVisible) {
            if (mEditor.beginSubPanel("Hierarchy", &mEditor.mHierarchyVisible, ImGuiDir_Left)) {

                Widgets::WidgetBase *root = mTopLevel;
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
                            mSelected = &mSettings.try_emplace(*hoveredWidget, *hoveredWidget, mEditor.getTool<Inspector>()).first->second;
                        else
                            mSelected = nullptr;
                    }
                } else {
                    ImGui::Text("Loading.");
                }
            }
            ImGui::End();
        }
    }

    bool WidgetFile::render()
    {
        constexpr float borderSize = 10.0f;

        bool open = true;

        if (mEditor.BeginResourceFile(this, mPath, mIsDirty, [this](const Filesystem::Path &path) { save(path); }, &open)) {

            ImVec2 pos;

            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Panels")) {

                    ImGui::MenuItem("Hierarchy", nullptr, &mEditor.mHierarchyVisible);
                    ImGui::MenuItem("Entity Details", nullptr, &mEditor.mWidgetDetailsVisible);

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            if (mEditor.beginContent()) {

                pos = ImGui::GetWindowPos();
                ImVec2 min = ImGui::GetWindowContentRegionMin();
                ImVec2 max = ImGui::GetWindowContentRegionMax();
                ImVec2 size = max - min;
                pos += min;
                InterfacesVector renderPos = static_cast<Window::OSWindow *>(ImGui::GetWindowViewport()->PlatformHandle)->renderPos();
                pos -= { static_cast<float>(renderPos.x), static_cast<float>(renderPos.y) };

                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 windowPos = mousePos - ImGui::GetWindowPos();

                Vector2i vSize { static_cast<int>(size.x), static_cast<int>(size.y) };

                mWidgetManager.injectPointerMove({ { static_cast<int>(windowPos.x), static_cast<int>(windowPos.y) }, { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) }, { 0, 0 } });

                if (mRenderTarget->resize(vSize)) {
                    mWidgetManager.onResize({ { 0, 0 }, vSize });
                }

                ImGui::Image((void *)mRenderTarget->texture()->resourceBlock(), size);

                ImGui::GetWindowDrawList()->PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

                Widgets::WidgetBase *hoveredWidget = nullptr;
                if (!mDragging)
                    hoveredWidget = mEditor.handleManagerInteractions(mWidgetManager, pos);
                if (hoveredWidget)
                    mSettings.try_emplace(hoveredWidget, hoveredWidget, mEditor.getTool<Inspector>());
                renderHierarchy(&hoveredWidget);
                renderSelection(pos, hoveredWidget);

                ImGuiIO &io = ImGui::GetIO();

                Rect2i screenSpace = mWidgetManager.getClientSpace();
                screenSpace.mTopLeft = { static_cast<int>(pos.x), static_cast<int>(pos.y) };

                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    screenSpace.mTopLeft += mWidgetManager.getScreenSpace().mTopLeft;

                if (mSelected) {

                    Vector2 mouse = ImGui::GetMousePos();
                    Vector2 dragDistance = mouse - Vector2 { io.MouseClickedPos[0] };

                    Widgets::WidgetBase *selectedWidget = mSelected->widget();

                    mEditor.renderWidgetBorders(selectedWidget, screenSpace.mTopLeft, IM_COL32(255, 255, 255, 255));

                    Vector3 absoluteSize = selectedWidget->getAbsoluteSize();
                    Vector2 absolutePos = selectedWidget->getAbsolutePosition() + Vector2 { screenSpace.mTopLeft };

                    Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

                    bool rightBorder = false, leftBorder = false, topBorder = false, bottomBorder = false;

                    bool hoveredWithBorder = selectedWidget->containsPoint(mouse, screenSpace, borderSize) && ImGui::IsWindowHovered();

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

                        if (rightBorder || leftBorder || topBorder || bottomBorder) {
                            hoveredWidget = selectedWidget;
                        }

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
                            if (hoveredWithBorder && selectedWidget == mWidgetManager.hoveredWidget())
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                        }
                    }

                    if (mMouseDown && dragDistance.length() >= io.MouseDragThreshold && !mDragging) {
                        mSelected->saveGeometry();
                        mDragging = true;
                    }

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

                Widgets::WidgetBase *pointerEventTargetWidget = mWidgetManager.pointerEventTargetWidget();
                if (pointerEventTargetWidget)
                    WidgetEditor::renderWidgetBorders(pointerEventTargetWidget, screenSpace.mTopLeft, IM_COL32(127, 100, 10, 255));

                Widgets::WidgetBase *focusedWidget = mWidgetManager.focusedWidget();
                if (focusedWidget)
                    WidgetEditor::renderWidgetBorders(focusedWidget, screenSpace.mTopLeft, IM_COL32(255, 200, 10, 255));

                if (hoveredWidget) {
                    WidgetSettings *hoveredSettings = &mSettings.try_emplace(hoveredWidget, hoveredWidget, mEditor.getTool<Inspector>()).first->second;

                    if (!mDragging && hoveredSettings != mSelected) {
                        WidgetEditor::renderWidgetBorders(hoveredWidget, screenSpace.mTopLeft, IM_COL32(127, 127, 127, 255));
                    }

                    if (io.MouseReleased[0] && !mDragging) {
                        mSelected = hoveredSettings;
                    }
                }

                ImGui::GetWindowDrawList()->PopClipRect();
            }
            ImGui::End();
        }
        ImGui::End();

        return open;
    }

}
}
