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

#include "Madgine/behaviorcollector.h"

#include "Madgine_Tools/debugger/debuggerview.h"

#include "Madgine/parametertuple.h"

#include "Madgine_Tools/behaviortool.h"

#include "Meta/serialize/formats.h"

UNIQUECOMPONENT(Engine::Tools::SceneEditor);

METATABLE_BEGIN_BASE(Engine::Tools::SceneEditor, Engine::Tools::ToolBase)
READONLY_PROPERTY(Views, views)
//READONLY_PROPERTY(CurrentScene, currentSceneFile)
METATABLE_END(Engine::Tools::SceneEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::SceneEditor, Engine::Tools::ToolBase)
ENCAPSULATED_FIELD(mCurrentSceneFile, currentSceneFile, openScene)
SERIALIZETABLE_END(Engine::Tools::SceneEditor)

namespace Engine {
namespace Tools {

    static std::map<std::string_view, std::string_view> sComponentIcons {
        { "Mesh", IMGUI_ICON_GRID },
        { "Transform", IMGUI_ICON_AXES },
        { "PointLight", IMGUI_ICON_POINTLIGHT },
        { "Skeleton", IMGUI_ICON_SKELETON }
    };

    SceneEditor::SceneEditor(ImRoot &root)
        : Tool<SceneEditor>(root)
        , mWindow(static_cast<const ClientImRoot &>(root).window())
    {
    }

    Threading::Task<bool> SceneEditor::init()
    {
        App::Application *app = App::Application::getSingletonPtr();
        if (!app)
            co_return false;
        mSceneMgr = &app->getGlobalAPIComponent<Scene::SceneManager>();
        mInspector = &mRoot.getTool<Inspector>();
        mSceneViews.emplace_back(std::make_unique<SceneView>(this, mWindow.getRenderer()));

        mSceneMgr->pause();
        mMode = STOP;

        mStartBuffer.resize(65536);

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> SceneEditor::finalize()
    {
        mSceneViews.clear();

        co_await ToolBase::finalize();
    }

    void SceneEditor::render()
    {
        auto guard = mSceneMgr->mutex().lock(AccessMode::WRITE);
        updateEntityCache();
        renderHierarchy();
        renderToolbar();
        renderSelection();
        std::erase_if(mSceneViews, [](const std::unique_ptr<SceneView> &view) { return !view->render(); });
        im3DInteractions();
        handleInputs();
    }

    void SceneEditor::renderMenu()
    {
        ToolBase::renderMenu();
        if (mVisible) {

            if (ImGui::BeginMenu("SceneEditor")) {

                bool isStopped = mMode == STOP;

                if (ImGui::MenuItem("Open", nullptr, nullptr, isStopped)) {
                    Execution::detach(mRoot.filePicker()
                        | Execution::then([this](DialogResult result, const Filesystem::Path &path) {
                              if (result == DialogResult::Accepted) {
                                  openScene(path);
                              }
                          }));
                }

                if (ImGui::MenuItem("Save", nullptr, nullptr, isStopped)) {
                    if (mCurrentSceneFile.empty())
                        saveScenePopup();
                    else
                        saveScene(mCurrentSceneFile);
                }
                if (ImGui::MenuItem("Save As...", nullptr, nullptr, isStopped))
                    saveScenePopup();

                ImGui::Separator();

                if (ImGui::MenuItem("Add View")) {
                    mSceneViews.emplace_back(std::make_unique<SceneView>(this, mWindow.getRenderer()));
                }

                ImGui::EndMenu();
            }
        }
    }

    void SceneEditor::renderSettings()
    {
        ImGui::BeginGroupPanel("Scene Editor");

        ImGui::Text("Bone-Forward");
        ImGui::SameLine();
        ImGui::DragFloat4("bone-forward", &mBoneForward.x);
        ImGui::DragFloat("Default Bone Length", &mDefaultBoneLength);
        ImGui::Checkbox("Show Bone Names", &mShowBoneNames);
        ImGui::Checkbox("Render 3D-Cursor", &mRender3DCursor);

        ImGui::EndGroupPanel();
    }

    std::string_view SceneEditor::key() const
    {
        return "SceneEditor";
    }

    const Filesystem::Path &SceneEditor::currentSceneFile() const
    {
        return mCurrentSceneFile;
    }

    int SceneEditor::hoveredAxis() const
    {
        return mHoveredAxis;
    }

    const Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> &SceneEditor::hoveredTransform() const
    {
        return mHoveredTransform;
    }

    void SceneEditor::deselect()
    {
        mSelectedEntity = {};
        mSelectedCamera = nullptr;
        mHoveredAxis = -1;
    }

    void SceneEditor::select(Render::Camera *camera)
    {
        mSelectedEntity = {};
        mSelectedCamera = camera;
        mHoveredAxis = -1;
    }

    void SceneEditor::select(const Scene::Entity::EntityPtr &entity)
    {
        mSelectedEntity = entity;
        mSelectedCamera = nullptr;
        mHoveredAxis = -1;
    }

    Scene::SceneManager &SceneEditor::sceneMgr()
    {
        return *mSceneMgr;
    }

    void SceneEditor::play()
    {
        if (mMode == PLAY)
            return;

        if (mMode == STOP) {
            //Memory::MemoryManager mgr { "Tmp" };
            //Serialize::FormattedSerializeStream out = mgr.openWrite(mStartBuffer, Serialize::Formats::safebinary);
            //Serialize::write(out, *mSceneMgr, "Scene");
        }

        mSceneMgr->unpause();
        mMode = PLAY;
    }

    void SceneEditor::pause()
    {
        if (mMode != PLAY)
            return;

        mSceneMgr->pause();
        mMode = PAUSE;
    }

    void SceneEditor::stop()
    {
        if (mMode == STOP)
            return;

        if (mMode == PLAY)
            mSceneMgr->pause();
        mMode = STOP;

        //Memory::MemoryManager mgr { "Tmp" };
        //Serialize::FormattedSerializeStream in = mgr.openRead(mStartBuffer, Serialize::Formats::safebinary);
        //Serialize::StreamResult result = Serialize::read(in, *mSceneMgr, nullptr, {}, Serialize::StateTransmissionFlags_ApplyMap | Serialize::StateTransmissionFlags_Activation);
        //if (result.mState != Serialize::StreamState::OK) {
        //LOG_ERROR(*result.mError);
        //}
    }

    void SceneEditor::openScene(const Filesystem::Path &p)
    {
        if (p.empty())
            return;

        mCurrentSceneFile = p;

        //auto guard = mSceneMgr->mutex().lock(AccessMode::WRITE);

        Filesystem::FileManager mgr { "Scene" };
        Serialize::FormattedSerializeStream in = mgr.openRead(mCurrentSceneFile, Serialize::Formats::xml);
        Serialize::StreamResult result = Serialize::read(in, mSceneMgr->container("Default"), nullptr, {}, Serialize::StateTransmissionFlags_ApplyMap | Serialize::StateTransmissionFlags_Activation);
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(*result.mError);
        }
    }

    void SceneEditor::saveScene(const Filesystem::Path &p)
    {
        mCurrentSceneFile = p;

        auto guard = mSceneMgr->mutex().lock(AccessMode::READ);

        Filesystem::FileManager mgr { "Scene" };
        Serialize::FormattedSerializeStream out = mgr.openWrite(mCurrentSceneFile, Serialize::Formats::xml);
        Serialize::write(out, mSceneMgr->container("Default"), "Scene");
    }

    int SceneEditor::createViewIndex()
    {
        return ++mRunningViewIndex;
    }

    void SceneEditor::renderSelection()
    {
        if (ImGui::Begin("SceneEditor", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Right, 0.2f, false, ImGuiCond_FirstUseEver);

            if (mSelectedEntity)
                renderEntity(mSelectedEntity);
            if (mSelectedCamera)
                renderCamera(mSelectedCamera);
        }
        ImGui::End();
    }

    void SceneEditor::renderHierarchy()
    {
        if (ImGui::Begin("SceneEditor - Hierarchy", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            if (ImGui::BeginPopupCompoundContextWindow()) {
                if (ImGui::MenuItem(IMGUI_ICON_PLUS " New Entity")) {
                    select(mSceneMgr->container("Default").createEntity());
                }
                ImGui::EndPopup();
            }

            for (EntityNode &entity : mEntityCache)
                renderHierarchyEntity(entity);
        }
        ImGui::End();
    }

    void SceneEditor::renderToolbar()
    {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);
        if (ImGui::Begin("SceneEditor - Toolbar", &mVisible, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Up, 0.01f, true, ImGuiCond_FirstUseEver);

            auto pre = [](bool b) { if (b) ImGui::BeginDisabled(); };
            auto post = [](bool b) { if (b) ImGui::EndDisabled(); };

            bool b = mMode == PLAY;
            pre(b);
            if (ImGui::Button(mMode == PAUSE ? "Resume" : IMGUI_ICON_PLAY)) {
                play();
            }
            post(b);

            ImGui::SameLine(0, 0);

            b = mMode == PAUSE || mMode == STOP;
            pre(b);
            if (ImGui::Button(IMGUI_ICON_PAUSE)) {
                pause();
            }
            post(b);

            ImGui::SameLine(0, 0);

            b = mMode == STOP;
            pre(b);
            if (ImGui::Button(IMGUI_ICON_STOP)) {
                stop();
            }
            post(b);
        }
        ImGui::End();
    }

    void SceneEditor::renderHierarchyEntity(EntityNode &node)
    {

        std::string &name = node.mEntity->mName;

        bool hovered = mSelectedEntity == node.mEntity;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        if (hovered)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (node.mChildren.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::EditableTreeNode(&*node.mEntity, &name, flags);
        bool aborted = false;

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            select(node.mEntity);
        }

        if (ImGui::BeginPopupCompoundContextItem()) {
            if (ImGui::MenuItem(IMGUI_ICON_X " Delete", "del")) {
                node.mEntity->endLifetime();
            }
            ImGui::EndPopup();
        }

        ImGui::DraggableValueTypeSource(name, node.mEntity);

        if (open) {
            for (EntityNode &node : node.mChildren)
                renderHierarchyEntity(node);
            ImGui::TreePop();
        }
    }

    void SceneEditor::eraseNode(EntityNode &node)
    {
        for (EntityNode &child : node.mChildren)
            eraseNode(child);
        mEntityMapping.erase(node.mEntity);
    }

    void SceneEditor::renderEntity(Scene::Entity::EntityPtr &entity)
    {
        if (entity.isDead())
            return;

        bool showParameters = false;
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
                if (BehaviorHandle behavior = BehaviorSelector()) {
                    entity->behaviors().addBehavior(std::move(behavior));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Temp Behavior")) {
                if (BehaviorHandle behavior = BehaviorSelector()) {
                    mPendingBehavior.mTargetEntity = entity;
                    mPendingBehavior.mHandle = behavior;
                    mPendingBehavior.mFuture = behavior.createParameters();
                    mPendingBehavior.mParameters.reset();
                    showParameters = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        if (showParameters)
            ImGui::OpenPopup("BehaviorParameters");

        if (ImGui::BeginPopup("BehaviorParameters")) {
            if (!mPendingBehavior.mFuture.is_ready()) {
                ImGui::Text("Loading...");
            } else {
                if (!mPendingBehavior.mParameters)
                    mPendingBehavior.mParameters = mPendingBehavior.mFuture;
                if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_SizingStretchProp)) {
                    mInspector->drawMembers(&mPendingBehavior.mParameters);
                    ImGui::EndTable();
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Create Behavior")) {
                    mPendingBehavior.mTargetEntity->addBehavior(mPendingBehavior.mHandle.create(mPendingBehavior.mParameters));
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

        IndexType<uint32_t> componentToRemove;
        for (const Scene::Entity::EntityComponentPtr<Scene::Entity::EntityComponentBase> &component : entity->components()) {
            std::string label = std::string { component.name() };
            auto it = sComponentIcons.find(label);
            if (it != sComponentIcons.end())
                label = std::string { it->second } + " " + label;
            ImGui::BeginGroupPanel(label.c_str());
            if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_Resizable)) {
                mInspector->drawMembers(component.getTyped());
                ImGui::EndTable();
            }

            ImGui::ItemSize({ ImGui::GetItemRectSize().x, 0 });

            ImGui::EndGroupPanel();

            if (ImGui::BeginPopupCompoundContextItem()) {
                if (ImGui::MenuItem((IMGUI_ICON_X " Delete " + std::string { component.name() }).c_str())) {
                    componentToRemove = component.type();
                }
                ImGui::EndPopup();
            }
        }
        if (componentToRemove) {
            entity->removeComponent(componentToRemove);
        }

        getTool<DebuggerView>().renderLifetime(entity->lifetime());

        getTool<BehaviorTool>().drawBehaviorList(entity->behaviors());

        if (Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> t = entity->getComponent<Scene::Entity::Transform>()) {
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
    }

    void SceneEditor::renderCamera(Render::Camera *camera)
    {
        mInspector->drawMembers(camera);
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

    void SceneEditor::updateEntityCache()
    {
        //Update + Remove deleted Entities
        mEntityCache.remove_if([this](EntityNode &node) { return updateEntityCache(node); });

        //Add missing Entities
        for (Scene::Entity::EntityPtr entity : mSceneMgr->container("Default").entities()) {
            if (!mEntityMapping.count(entity))
                createEntityMapping(std::move(entity));
        }
    }

    bool SceneEditor::updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent)
    {
        if (node.mEntity.isDead() || (!node.mEntity->hasComponent<Scene::Entity::Transform>() && parent) || (node.mEntity->hasComponent<Scene::Entity::Transform>() && ((parent && parent->getComponent<Scene::Entity::Transform>() != node.mEntity->getComponent<Scene::Entity::Transform>()->parent()) || (!parent && node.mEntity->getComponent<Scene::Entity::Transform>()->parent())))) {
            eraseNode(node);
            return true;
        }
        node.mChildren.remove_if([&](EntityNode &childNode) { return updateEntityCache(childNode, node.mEntity); });
        return false;
    }

    void SceneEditor::createEntityMapping(Scene::Entity::EntityPtr e)
    {
        Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = e->getComponent<Scene::Entity::Transform>();

        Scene::Entity::EntityPtr parent;

        if (transform) {
            Scene::Entity::Transform *parentTransform = transform->parent();
            if (parentTransform) {
                for (Scene::Entity::EntityPtr p : mSceneMgr->container("Default").entities()) {
                    if (p->getComponent<Scene::Entity::Transform>() == parentTransform) {
                        parent = p;
                        break;
                    }
                }
            }
        }

        if (parent) {
            if (!mEntityMapping.count(parent))
                createEntityMapping(parent);
        }

        std::list<EntityNode> &container = parent ? mEntityMapping[parent]->mChildren : mEntityCache;

        container.push_back({ std::move(e) });
        mEntityMapping[e] = &container.back();
    }

    void SceneEditor::im3DInteractions()
    {
        for (EntityNode &node : mEntityCache) {
            Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> transform = node.mEntity->getComponent<Engine::Scene::Entity::Transform>();
            if (transform) {
                bool selected = mSelectedEntity == node.mEntity;

                if (ImGui::BeginDragDropTarget()) {
                    Scene::Entity::EntityPtr *newChild;
                    if (ImGui::AcceptDraggableValueType(newChild, nullptr, [](Scene::Entity::EntityPtr *child) { return (*child)->hasComponent<Scene::Entity::Transform>(); })) {
                        Scene::Entity::EntityComponentPtr<Engine::Scene::Entity::Transform> childTransform = (*newChild)->getComponent<Engine::Scene::Entity::Transform>();
                        assert(childTransform);
                        childTransform->setParent(transform);
                    }
                    ImGui::EndDragDropTarget();
                }

                Matrix4 transformM = transform->worldMatrix();
                AABB bb = { { -0.2f, -0.2f, -0.2f }, { 0.2f, 0.2f, 0.2f } };
                if (node.mEntity->hasComponent<Scene::Entity::Mesh>() && node.mEntity->getComponent<Scene::Entity::Mesh>()->data())
                    bb = node.mEntity->getComponent<Scene::Entity::Mesh>()->aabb();

                Im3DBoundingObjectFlags flags = Im3DBoundingObjectFlags_ShowOnHover;
                if (selected)
                    flags |= Im3DBoundingObjectFlags_ShowOutline;

                if (Im3D::BoundingBox(node.mEntity->mName.c_str(), bb, transformM, flags)) {
                    if (ImGui::IsMouseClicked(0)) {
                        select(node.mEntity);
                    }
                }
            }
        }

        const Ray3 &ray = Im3D::GetMouseRay();

        if (mRender3DCursor)
            Im3D::Arrow3D(IM3D_LINES, 0.3f, ray.point(10.0f), ray.point(20.0f));
    }

    void SceneEditor::saveScenePopup()
    {
        Execution::detach(mRoot.filePicker(true) | Execution::then([this](DialogResult result, const Filesystem::Path &path) {
            if (result == DialogResult::Accepted) {
                saveScene(path);
            }
        }));
    }
}
}
