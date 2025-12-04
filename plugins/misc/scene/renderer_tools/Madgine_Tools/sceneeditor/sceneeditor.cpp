#include "../scenerenderertoolslib.h"

#include "sceneeditor.h"

#include "Madgine/window/mainwindow.h"
#include "Madgine_Tools/imgui/clientimroot.h"

#include "Madgine_Tools/imguiicons.h"
#include "im3d/im3d.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/app/application.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/entity/entitycomponentcollector.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "Meta/math/boundingbox.h"

#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/serialize/memory/memorymanager.h"
#include "Meta/serialize/hierarchy/statetransmissionflags.h"

#include "Interfaces/input/inputevents.h"

#include "Madgine/behavior/behaviorcollector.h"

#include "Madgine_Tools/debugger/debuggerview.h"

#include "Madgine/behavior/parametertuple.h"

#include "Madgine_Tools/behaviortool.h"

#include "Meta/serialize/formats.h"

#include "Madgine/render/scenemainwindowcomponent.h"

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
        : mTool(tool)
        , mEntityCache(*this)
    {
    }

    Behavior::BehaviorHandle SceneEditor::render()
    {
        renderToolBar();

        auto guard = sceneMgr().mutex().lock(AccessMode::WRITE);
        mEntityCache.update();
        renderHierarchy();
        Behavior::BehaviorHandle behaviorToAdd = renderDetails();
        std::erase_if(mSceneViews, [](const std::unique_ptr<SceneView> &view) { return !view->render(); });
        im3DInteractions();
        handleInputs();
        return behaviorToAdd;
    }

    int SceneEditor::hoveredAxis() const
    {
        return mHoveredAxis;
    }

    Scene::Entity::Transform *const &SceneEditor::hoveredTransform() const
    {
        return mHoveredTransform;
    }

    void SceneEditor::deselect()
    {
        mSelectedEntity = {};
        mHoveredAxis = -1;
    }

    void SceneEditor::select(const Scene::Entity::EntityPtr &entity)
    {
        mSelectedEntity = entity;
        mHoveredAxis = -1;
    }

    std::string SceneEditor::patchIcon(std::string_view name)
    {
        std::string label = std::string { name };
        auto it = sComponentIcons.find(label);
        if (it != sComponentIcons.end())
            label = std::string { it->second } + " " + label;
        return label;
    }

    void SceneEditor::createView(Render::SceneRenderData &sceneData, Render::PointShadowRenderData &pointShadowRenderData, Im3D::Im3DContext *context)
    {
        mSceneViews.emplace_back(std::make_unique<SceneView>(*this, sceneData, pointShadowRenderData, context));
    }

    void SceneEditor::clearViews()
    {
        mSceneViews.clear();
    }

    Behavior::BehaviorHandle SceneEditor::renderDetails()
    {
        Behavior::BehaviorHandle behaviorToAdd;

        if (mTool.mEntityDetailsVisible) {
            if (mTool.beginSubPanel("Details", &mTool.mEntityDetailsVisible, ImGuiDir_Right)) {

                if (mSelectedEntity)
                    behaviorToAdd = renderEntity(mSelectedEntity);
            }
            ImGui::End();
        }

        return behaviorToAdd;
    }

    void SceneEditor::renderHierarchy()
    {
        if (mTool.mHierarchyVisible) {
            if (mTool.beginSubPanel("Hierarchy", &mTool.mHierarchyVisible, ImGuiDir_Left)) {

                if (ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::MenuItem(IMGUI_ICON_PLUS " New Entity")) {
                        select(sceneMgr().container("Default").createEntity());
                    }
                    ImGui::EndPopup();
                }

                for (const EntityCache::Node &entity : mEntityCache)
                    renderHierarchyEntity(entity, true);
            }
            ImGui::End();
        }
    }

    void SceneEditor::renderHierarchyEntity(const EntityCache::Node &node, bool visible)
    {

        bool success = node.mEntity.access([&](Scene::Entity::Entity &e) {
            
            Scene::Entity::Transform *transform = e.getComponent<Engine::Scene::Entity::Transform>();

            if (visible) {

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
                if (mSelectedEntity == node.mEntity)
                    flags |= ImGuiTreeNodeFlags_Selected;

                if (node.mChildren.empty())
                    flags |= ImGuiTreeNodeFlags_Leaf;

                bool open = ImGui::EditableTreeNode(&e, &e.mName, flags);

                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                    select(node.mEntity);
                }

                if (ImGui::BeginPopupCompoundContextItem()) {
                    if (ImGui::MenuItem(IMGUI_ICON_X " Delete", "del")) {
                        e.endLifetime();
                    }
                    ImGui::EndPopup();
                }

                ImGui::DraggableValueTypeSource(e.mName, node.mEntity);

                if (transform) {

                    if (ImGui::BeginDragDropTarget()) {
                        Scene::Entity::EntityPtr newChild;
                        if (ImGui::AcceptDraggableValueType(newChild, [](const auto &child) { return Execution::access_binding(child, [](Scene::Entity::Entity &e) { return e.hasComponent<Scene::Entity::Transform>(); }); })) {
                            newChild.access([&](Scene::Entity::Entity &childEntity) {
                                Engine::Scene::Entity::Transform *childTransform = childEntity.getComponent<Engine::Scene::Entity::Transform>();
                                assert(childTransform);
                                childTransform->setParent(node.mEntity);
                                return true;
                            });
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                for (const EntityCache::Node &node : node.mChildren)
                    renderHierarchyEntity(node, open);
                if (open)
                    ImGui::TreePop();
            } else {
                for (const EntityCache::Node &node : node.mChildren)
                    renderHierarchyEntity(node, false);
            }

            if (transform){
                Matrix4 transformM = transform->worldMatrix();
                AABB bb = { { -0.2f, -0.2f, -0.2f }, { 0.2f, 0.2f, 0.2f } };
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

    Behavior::BehaviorHandle SceneEditor::renderEntity(Scene::Entity::EntityPtr &e)
    {
        Behavior::BehaviorHandle behaviorToAdd;

        e.access([&](Scene::Entity::Entity &entity) {
            if (ImGui::BeginPopupCompoundContextWindow()) {
                if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Component")) {
                    for (auto [name, index] : Scene::Entity::EntityComponentRegistry::sComponentsByName()) {
                        if (!entity.hasComponent(name)) {
                            if (ImGui::MenuItem(name.data())) {
                                entity.addComponent(index);
                                if (name == "Transform") {
                                    entity.getComponent<Scene::Entity::Transform>()->mPosition = { 0, 0, 0 };
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

            IndexType<uint32_t> componentToRemove;
            for (const Scene::Entity::EntityComponentHandle &component : entity.components()) {
                ImGui::BeginGroupPanel(patchIcon(component.name()).c_str());
                if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_Resizable)) {
                    mTool.mInspector->drawMembers(component.getTyped());
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
                entity.removeComponent(componentToRemove);
            }

            mTool.getTool<DebuggerView>().renderLifetime(entity.lifetime());

            mTool.getTool<BehaviorTool>().drawBehaviorList(entity.behaviors());

            if (Scene::Entity::Transform *t = entity.getComponent<Scene::Entity::Transform>()) {
                constexpr Color4 colors[] = {
                    { 0.5f, 0, 0, 0.7f },
                    { 0, 0.5f, 0, 0.7f },
                    { 0, 0, 0.5f, 0.7f }
                };
                constexpr Vector3 offsets[] = {
                    { 1, 0, 0 },
                    { 0, 1, 0 },
                    { 0, 0, 1 }
                };

                const char *labels[] = {
                    "x-move",
                    "y-move",
                    "z-move"
                };

                mHoveredAxis = -1;
                mHoveredTransform = {};

                Vector3 pos = (t->worldMatrix() * Vector4::UNIT_W).xyz();

                for (size_t i = 0; i < 3; ++i) {
                    Im3D::Arrow3D(IM3D_TRIANGLES, 0.1f, pos, pos + offsets[i], { .mColor = colors[i] });
                    if (Im3D::BoundingBox(labels[i], 0, 2)) {
                        mHoveredAxis = i;
                        mHoveredTransform = t;
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
            const Ray3 &ray = Im3D::GetMouseRay();
            Im3D::Arrow3D(IM3D_LINES, 0.3f, ray.point(10.0f), ray.point(20.0f));
        }
    }

    SceneTool &SceneEditor::tool()
    {
        return mTool;
    }

    void SceneEditor::renderToolBar()
    {
        if (mTool.beginToolBar("Scene")) {

            if (ImGui::Button(IMGUI_ICON_PLUS)) {
                select(sceneMgr().container("Editor").createEntity());
            }

            mTool.endToolBar();
        }
    }

}
}
