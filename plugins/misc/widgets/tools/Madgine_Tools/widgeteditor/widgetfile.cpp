#include "../widgetstoolslib.h"

#include "widgetfile.h"

#include "Meta/math/bounds.h"
#include "Meta/serialize/container/container_operations.h"
#include "Meta/serialize/formats.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/texture.h"
#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/widgets/util/widgetsrenderdata.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/util/trace_imgui.h"
#include "imgui/imguiaddons.h"
#include "widgeteditor.h"

METATABLE_BEGIN(Engine::Tools::WidgetFile)
    READONLY_PROPERTY(WidgetManager, widgetManager)
METATABLE_END(Engine::Tools::WidgetFile)

namespace Engine {
namespace Tools {

    WidgetFile::WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Resource *resource)
        : ResourceFile(editor, resource ? resource->path() : "")
        , mWidgetManager(editor.manager())
        , mRenderTarget(editor.manager().window().getRenderer()->createRenderTexture({ 1, 1 }, { .mName { resource ? resource->name() : "<Unnamed>" }, .mFormat = Render::FORMAT_RGBA8_SRGB }))
    {
        static_cast<ClientImRoot &>(editor.root()).addRenderTarget(mRenderTarget.get());
        mRenderTarget->addRenderPass(&mWidgetManager);
        mWidgetManager.onResize({ { 0, 0 }, { 1, 1 } });

        mTopLevel = mWidgetManager.createTopLevel();

        if (resource) {
            Serialize::SerializeManager serializeMgr { "CompoundWidget" };
            Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), serializeMgr.wrapStream(resource->readAsStream(), true) };

            Serialize::StreamResult result = Serialize::read({ stream, CallerHierarchy { &mWidgetManager } }, *mTopLevel, "Widget");
            if (result.mState != Serialize::StreamState::OK) {
                LOG_ERROR(result);
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

    void WidgetFile::saveAs(const Filesystem::Path &path)
    {
        Filesystem::FileManager mgr { "Widget" };

        Serialize::FormattedSerializeStream stream = mgr.openWrite(path, Serialize::Formats::xml);

        Serialize::write(stream, *mTopLevel, "Widget");

        mPath = path;

        mHistory.onSave();
    }

    void WidgetFile::renderSelection()
    {
        if (editor().mWidgetDetailsVisible) {
            if (editor().beginSubPanel("Details", &editor().mWidgetDetailsVisible, ImGuiDir_Right)) {

                if (mSelected) {
                    mSelected->render(mHistory);
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
                for (const auto &[name, index] : Widgets::WidgetRegistry::sComponentsByName()) {
                    if (ImGui::MenuItem(name.data())) {
                        const Widgets::WidgetRegistry::Annotations &annotation = Widgets::WidgetRegistry::get(index);
                        w->createChildByAnnotation(annotation);
                    }
                }
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

            ImGui::DraggableValueTypeSource<Widgets::WidgetBase *>(w->mName, TracedRoot<Widgets::WidgetBase *const &> { mHistory, w });
            if (ImGui::BeginDragDropTarget()) {
                Widgets::WidgetBase *newChild = nullptr;
                if (ImGui::AcceptDraggableValueType(newChild, [](const Traced<Widgets::WidgetBase *const &> &child) { return child.get()->getParent() ? KeyValueResult {} : KEYVALUE_UNKNOWN_ERROR(); })) {
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
        if (editor().mHierarchyVisible) {
            if (editor().beginSubPanel("Hierarchy", &editor().mHierarchyVisible, ImGuiDir_Left)) {

                Widgets::WidgetBase *root = mTopLevel;
                if (root) {
                    if (ImGui::BeginPopupCompoundContextWindow()) {
                        if (ImGui::BeginMenu(IMGUI_ICON_PLUS " New Widget")) {
                            for (const auto &[name, index] : Widgets::WidgetRegistry::sComponentsByName()) {
                                if (ImGui::MenuItem(name.data())) {
                                    const Widgets::WidgetRegistry::Annotations &annotation = Widgets::WidgetRegistry::get(index);
                                    root->createChildByAnnotation(annotation);
                                }
                            }
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

    void WidgetFile::render()
    {
        constexpr float borderSize = 10.0f;

        bool open = true;

        if (Begin(&open)) {

            ImVec2 pos;

            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Panels")) {

                    ImGui::MenuItem("Hierarchy", nullptr, &editor().mHierarchyVisible);
                    ImGui::MenuItem("Widget Details", nullptr, &editor().mWidgetDetailsVisible);

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            if (editor().beginContent()) {

                pos = ImGui::GetWindowPos();
                ImVec2 min = ImGui::GetWindowContentRegionMin();
                ImVec2 max = ImGui::GetWindowContentRegionMax();
                ImVec2 size = max - min;
                pos += min;
                ImVec2 renderPos = ImGui::GetWindowViewport()->Pos;
                pos -= renderPos;

                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 windowPos = mousePos - ImGui::GetWindowPos();

                Vector2i vSize { static_cast<int>(size.x), static_cast<int>(size.y) };

                mWidgetManager.injectPointerMove({ { static_cast<int>(windowPos.x), static_cast<int>(windowPos.y) }, { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) }, { 0, 0 } });

                if (vSize.x > 0 && vSize.y > 0 && mRenderTarget->resize(vSize)) {
                    mWidgetManager.onResize({ { 0, 0 }, vSize });
                }

                ImGui::Image((void *)mRenderTarget->texture()->resourceBlock(), size);

                ImGui::GetWindowDrawList()->PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

                ImGui::InteractiveViewResultFlags result = ImGui::InteractiveView(ImGui::GetID("View"));
                int mouseButton = (result & ImGui::InteractiveViewResultFlags_MouseButtonMask_) - 1;

                bool dragging = result & ImGui::InteractiveViewResultFlags_Dragging;

                Widgets::WidgetBase *hoveredWidget = nullptr;
                if (!dragging)
                    hoveredWidget = editor().handleManagerInteractions(mWidgetManager, pos);
                if (hoveredWidget)
                    mSettings.try_emplace(hoveredWidget, hoveredWidget, mEditor.getTool<Inspector>());
                renderHierarchy(&hoveredWidget);
                renderSelection();

                ImGuiIO &io = ImGui::GetIO();

                Rect2i screenSpace = mWidgetManager.getClientSpace();
                screenSpace.mTopLeft = { static_cast<int>(pos.x), static_cast<int>(pos.y) };

                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    screenSpace.mTopLeft += mWidgetManager.getScreenSpace().mTopLeft;

                if (mSelected) {

                    Vector2 mouse = ImGui::GetMousePos();
                    Vector2 dragDistance = mouse - Vector2 { io.MouseClickedPos[0] };

                    Widgets::WidgetBase *selectedWidget = mSelected->widget();

                    editor().renderWidgetBorders(selectedWidget, screenSpace.mTopLeft, IM_COL32(255, 255, 255, 255));

                    Vector3 absoluteSize = selectedWidget->getAbsoluteSize();
                    Vector2 absolutePos = selectedWidget->getAbsolutePosition() + Vector2 { screenSpace.mTopLeft };

                    Bounds bounds(absolutePos.x, absolutePos.y + absoluteSize.y, absolutePos.x + absoluteSize.x, absolutePos.y);

                    bool rightBorder = false, leftBorder = false, topBorder = false, bottomBorder = false;

                    bool hoveredWithBorder = selectedWidget->containsPoint(mouse, screenSpace, borderSize) && ImGui::IsItemHovered();

                    if (!dragging && hoveredWithBorder) {

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
                    }

                    if (ImGui::IsItemActivated()) {
                        if (!hoveredWithBorder) {
                            mSelected = nullptr;
                        } else {
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

                    if (result & ImGui::InteractiveViewResultFlags_DragStarted) {
                        mSelected->saveGeometry();
                    }

                    enum ResizeMode {
                        RELATIVE,
                        ABSOLUTE
                    };

                    ResizeMode resizeMode = RELATIVE;
                    if (io.KeyShift) {
                        resizeMode = ABSOLUTE;
                    }

                    if (dragging) {

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

                        if (result & ImGui::InteractiveViewResultFlags_DragStopped) {
                            mSelected->applyGeometry();
                        }
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

                    if (!dragging && hoveredSettings != mSelected) {
                        WidgetEditor::renderWidgetBorders(hoveredWidget, screenSpace.mTopLeft, IM_COL32(127, 127, 127, 255));
                    }

                    if ((result & ImGui::InteractiveViewResultFlags_Pressed) && mouseButton == 0) {
                        mSelected = hoveredSettings;
                    }
                }

                ImGui::GetWindowDrawList()->PopClipRect();
            }
            ImGui::End();
        }
        ImGui::End();

        if (!open) {
            if (mHistory.isDirty()) {
                mEditor.root().dialogs().showGrouped("Close", closeDialog(), [this]() { mCloseRequested = true; });
            } else {
                mCloseRequested = true;
            }
        }
    }

    Widgets::WidgetManager &WidgetFile::widgetManager()
    {
        return mWidgetManager;
    }

}
}
