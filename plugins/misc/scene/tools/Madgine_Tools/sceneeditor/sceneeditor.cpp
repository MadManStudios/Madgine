#include "../scenerenderertoolslib.h"

#include "sceneeditor.h"

#include "Platform/input/inputevents.h"

#include "Meta/math/boundingbox.h"
#include "Meta/serialize/formats.h"

#include "Madgine/app/application.h"
#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/render/scenemainwindowcomponent.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/entity/entitycomponentcollector.h"
#include "Madgine/scene/scenemanager.h"
#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/serialize/memory/memorymanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/behaviortool.h"
#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/imgui/clientimroot.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/util/trace_imgui.h"
#include "im3d/im3d.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "scenetool.h"

namespace Engine {
namespace Tools {

    static std::map<std::string_view, std::string_view> sComponentIcons {
        { "Mesh", IMGUI_ICON_GRID },
        { "Transform", IMGUI_ICON_AXES },
        { "PointLight", IMGUI_ICON_POINTLIGHT },
        { "Skeleton", IMGUI_ICON_SKELETON }
    };

    SceneEditor::SceneEditor(SceneTool &tool)
        : mEntityCache(*this)
        , mTool(tool)
    {
    }

    Behavior::BehaviorHandle SceneEditor::render(UndoStack &history, std::vector<std::unique_ptr<SceneView>> &views)
    {
        renderToolBar();

        auto guard = sceneMgr().mutex().lock(AccessMode::WRITE);
        mEntityCache.update();
        renderHierarchy(history);
        Behavior::BehaviorHandle behaviorToAdd = renderDetails(history);
        std::erase_if(views, [](const std::unique_ptr<SceneView> &view) { return !view->render(); });
        im3DInteractions();
        handleInputs();
        return behaviorToAdd;
    }

    IndexType<uint8_t> SceneEditor::hoveredAxis() const
    {
        return mHoveredAxis;
    }

    const Scene::Entity::EntityPtr &SceneEditor::hoveredEntity() const
    {
        return mHoveredEntity;
    }

    void SceneEditor::deselect()
    {
        mSelectedEntity = {};
        mHoveredAxis.reset();
    }

    void SceneEditor::select(const Scene::Entity::EntityPtr &entity)
    {
        mSelectedEntity = entity;
        mHoveredAxis.reset();
    }

    std::string SceneEditor::patchIcon(std::string_view name)
    {
        std::string label = std::string { name };
        auto it = sComponentIcons.find(label);
        if (it != sComponentIcons.end())
            label = std::string { it->second } + " " + label;
        return label;
    }

    Behavior::BehaviorHandle SceneEditor::renderDetails(UndoStack &history)
    {
        Behavior::BehaviorHandle behaviorToAdd;

        if (mTool.mEntityDetailsVisible) {
            if (mTool.beginSubPanel("Details", &mTool.mEntityDetailsVisible, ImGuiDir_Right)) {

                if (mSelectedEntity)
                    behaviorToAdd = renderEntity(mSelectedEntity, history);
            }
            ImGui::End();
        }

        return behaviorToAdd;
    }

    void SceneEditor::renderHierarchy(UndoStack &history)
    {
        if (mTool.mHierarchyVisible) {
            if (mTool.beginSubPanel("Hierarchy", &mTool.mHierarchyVisible, ImGuiDir_Left)) {

                if (ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::MenuItem(IMGUI_ICON_PLUS " New Entity")) {
                        sceneMgr().container("Editor").createEntity("", {}, {}, [this](Scene::Entity::EntityPtr ptr) { select(std::move(ptr)); });
                    }
                    ImGui::EndPopup();
                }

                bool mCurrentContainerVisible = true;
                Scene::SceneContainer *currentContainer = nullptr;

                for (const EntityCache::Node &entity : mEntityCache) {

                    if (mEntityCache.sortingFlags() & EntityCacheSortingFlags::GroupByContainer) {
                        Execution::access_binding(entity.mEntity, [&](Scene::Entity::Entity &e) {
                            if (&e.container() != currentContainer) {
                                currentContainer = &e.container();
                                mCurrentContainerVisible = ImGui::CollapsingHeader(currentContainer->name().data());
                            }
                        });
                    }

                    if (mCurrentContainerVisible)
                        renderHierarchyEntity(entity, true, history);
                }
            }
            ImGui::End();
        }
    }

    void SceneEditor::renderHierarchyEntity(const EntityCache::Node &node, bool visible, UndoStack &history)
    {

        bool success = Execution::access_binding(node.mEntity, [&](Scene::Entity::Entity &e) {
            Scene::Entity::Transform *transform = e.getComponent<Engine::Scene::Entity::Transform>();

            if (visible) {

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
                if (mSelectedEntity == node.mEntity)
                    flags |= ImGuiTreeNodeFlags_Selected;

                if (node.mChildren.empty())
                    flags |= ImGuiTreeNodeFlags_Leaf;

                bool open = ImGui::EditableTreeNode(&e, &e.mName, flags);

                if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)) {
                    select(node.mEntity);
                }

                if (ImGui::BeginPopupCompoundContextItem()) {
                    if (ImGui::MenuItem(IMGUI_ICON_X " Delete", "del")) {
                        e.endLifetime();
                    }
                    ImGui::EndPopup();
                }

                ImGui::DraggableValueTypeSource<Scene::Entity::EntityPtr>(e.mName, TracedRoot<const Scene::Entity::EntityPtr &> { history, node.mEntity });

                if (transform) {

                    if (ImGui::BeginDragDropTarget()) {
                        Scene::Entity::EntityPtr newChild;
                        if (ImGui::AcceptDraggableValueType(newChild, [&](const auto &child) -> Reflect::Result {
                                if (&child.undoStack() != &history) {
                                    return REFLECT_UNKNOWN_ERROR() << "Cannot drag/drop Entities from different SceneContainers";
                                }
                                return Execution::access_binding(child.get(), [](Scene::Entity::Entity &e) { return e.hasComponent<Scene::Entity::Transform>(); }) ? Reflect::Result {} : REFLECT_UNKNOWN_ERROR();
                            })) {
                            Execution::access_binding(newChild, [&](Scene::Entity::Entity &childEntity) {
                                childEntity.setParent(node.mEntity);
                            });
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                for (const EntityCache::Node &node : node.mChildren)
                    renderHierarchyEntity(node, open, history);
                if (open)
                    ImGui::TreePop();
            } else {
                for (const EntityCache::Node &node : node.mChildren)
                    renderHierarchyEntity(node, false, history);
            }

            if (transform) {
                Math::Matrix4 transformM = transform->worldMatrix(e);
                Math::AABB bb = { { -0.2f, -0.2f, -0.2f }, { 0.2f, 0.2f, 0.2f } };
                if (e.hasComponent<Scene::Entity::Mesh>() && e.getComponent<Scene::Entity::Mesh>()->data())
                    bb = e.getComponent<Scene::Entity::Mesh>()->aabb();

                Im3DBoundingObjectFlags flags = Im3DBoundingObjectFlags_ShowOnHover;
                if (mSelectedEntity == node.mEntity)
                    flags |= Im3DBoundingObjectFlags_ShowOutline;

                if (Im3D::BoundingBox(e.mName.c_str(), bb, transformM, flags)) {
                    if (ImGui::IsMouseClicked(0)) {
                        select(node.mEntity);
                    }
                }
            }
        });

        if (!success) {
            mEntityCache.eraseNode(node);
        }
    }

    Behavior::BehaviorHandle SceneEditor::renderEntity(Scene::Entity::EntityPtr &e, UndoStack &history)
    {
        Behavior::BehaviorHandle behaviorToAdd;

        TracedRoot<Scene::Entity::EntityPtr> entityPtr { history, e };

        Execution::access_binding(entityPtr, [&](const Traced<Scene::Entity::Entity &> &_entity) {            

            TracedContext<Scene::Entity::Entity &> entity = _entity;

            if (ImGui::BeginPopupCompoundContextWindow()) {
                if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Component")) {
                    for (auto [name, index] : Scene::Entity::EntityComponentRegistry::sComponentsByName()) {
                        if (!entity->hasComponent(name)) {
                            if (ImGui::MenuItem(name.data())) {
                                entity->addComponent(index);
                                if (name == "Transform") {
                                    entity->getComponent<Scene::Entity::Transform>()->mPosition = { 0, 0, 0 };
                                }
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Behavior")) {
                    behaviorToAdd = BehaviorSelector();
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            if (ImGui::CollapsingHeader("Components")) {
                IndexType<uint32_t> componentToRemove;
                for (const Scene::Entity::EntityComponentHandle &component : entity->components()) {

                    const Traced<Reflect::ScopePtr> &componentPtr = entity.trace([](Scene::Entity::Entity &e, uint32_t type) {
                        return Reflect::ScopePtr { e.getComponent(type), *Scene::Entity::EntityComponentRegistry::get(type).mType };
                    },
                        component.mType);

                    ImGui::BeginGroupPanel(patchIcon(component.name()).c_str());
                    if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_Resizable)) {
                        mTool.mInspector->drawMembers(componentPtr);
                        ImGui::EndTable();
                    }

                    ImGui::ItemSize({ ImGui::GetItemRectSize().x, 0 });

                    ImGui::EndGroupPanel();

                    if (ImGui::BeginPopupCompoundContextItem()) {
                        if (ImGui::MenuItem((IMGUI_ICON_X " Delete " + std::string { component.name() }).c_str())) {
                            componentToRemove = component.mType;
                        }
                        ImGui::EndPopup();
                    }
                }
                if (componentToRemove) {
                    entity->removeComponent(componentToRemove);
                }
            }

            if (ImGui::CollapsingHeader("Lifetime")) {
                mTool.getTool<DebuggerView>().renderLifetime(entity->lifetime());
            }

            if (ImGui::CollapsingHeader("Behaviors")) {
                ImGui::Dummy({ 0, 0 });
                if (ImGui::InlineContextButton("N", mTool.mBehaviorFlags & Reflect::AccessorFlags_Named)) {
                    mTool.mBehaviorFlags ^= Reflect::AccessorFlags_Named;
                }
                ImGui::SetItemTooltip("Show Named Parameters");

                mTool.mInspector->pushFlags(mTool.mBehaviorFlags);

                mTool.getTool<BehaviorTool>().drawBehaviorList(entity.trace(&Scene::Entity::Entity::behaviors));

                mTool.mInspector->popFlags();
            }

            if (Scene::Entity::Transform *t = entity->getComponent<Scene::Entity::Transform>()) {
                constexpr Math::Color4 colors[] = {
                    { 0.5f, 0, 0, 0.7f },
                    { 0, 0.5f, 0, 0.7f },
                    { 0, 0, 0.5f, 0.7f }
                };
                constexpr Math::Vector3 offsets[] = {
                    { 1, 0, 0 },
                    { 0, 1, 0 },
                    { 0, 0, 1 }
                };

                const char *labels[] = {
                    "x-move",
                    "y-move",
                    "z-move"
                };

                mHoveredAxis.reset();
                mHoveredEntity = {};

                Math::Vector3 pos = (t->worldMatrix(entity.get()) * Math::Vector4::UNIT_W).xyz();

                for (size_t i = 0; i < 3; ++i) {
                    Im3D::Arrow3D(IM3D_TRIANGLES, 0.1f, pos, pos + offsets[i], { .mColor = colors[i] });
                    if (Im3D::BoundingBox(labels[i], 0, 2)) {
                        mHoveredAxis = i;
                        mHoveredEntity = entity->pointer();
                    }
                }

                /* if (Scene::Entity::Skeleton *s = entity->getComponent<Scene::Entity::Skeleton>()) {
                    if (const Render::SkeletonDescriptor *skeleton = s->data()) {
                        for (size_t i = 0; i < skeleton->mBones.size(); ++i) {
                            const Engine::Render::Bone &bone = skeleton->mBones[i];

                            Matrix4 m = s->matrices()[i] * bone.mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                            Matrix4 world = t->worldMatrix();

                            if (mShowBoneNames)
                                Im3D::Text(bone.mName.c_str(), { .mTransform = world * m, .mFontSize = 2.0f });

                            Vector4 start = world * m * Vector4::UNIT_W;
                            Vector4 end;

                            if (bone.mFirstChild) {
                                Matrix4 m_child = s->matrices()[bone.mFirstChild] * skeleton->mBones[bone.mFirstChild].mOffsetMatrix.Inverse() * skeleton->mMatrix.Inverse();
                                end = world * m_child * Vector4::UNIT_W;
                            } else {
                                end = world * m * skeleton->mMatrix * (mBoneForward * mDefaultBoneLength) + (1.0f - mBoneForward.w) * start;
                            }
                            float length = (end - start).xyz().length();
                            Im3D::Arrow3D(IM3D_LINES, 0.1f * length, start.xyz(), end.xyz());
                        }
                    }
                }*/
            }
            return true;
        });

        return behaviorToAdd;
    }

    void SceneEditor::handleInputs()
    {
        return;
        /* if (ImGui::IsKeyPressed(Input::Key::Delete)) {
            if (mSelectedEntity) {
                mSelectedEntity->remove();
                mSelectedEntity.reset();
            }
        }*/
    }

    void SceneEditor::im3DInteractions()
    {

        if (mTool.mRender3DCursor) {
            const Math::Ray3 &ray = Im3D::GetMouseRay();
            Im3D::Arrow3D(IM3D_LINES, 0.3f, ray.point(10.0f), ray.point(20.0f));
        }
    }

    SceneTool &SceneEditor::tool()
    {
        return mTool;
    }

    void SceneEditor::renderToolBar()
    {
        if (ImGui::BeginToolBar("Scene")) {

            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_N);
            if (ImGui::Button(IMGUI_ICON_PLUS)) {
                sceneMgr().container("Editor").createEntity("", {}, {}, [this](Scene::Entity::EntityPtr ptr) { select(std::move(ptr)); });
            }

            ImGui::EndToolBar();
        }
    }

}
}
