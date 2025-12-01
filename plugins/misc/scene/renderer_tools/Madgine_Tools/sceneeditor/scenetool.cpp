#include "../scenerenderertoolslib.h"

#include "scenetool.h"

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

UNIQUECOMPONENT(Engine::Tools::SceneTool);

METATABLE_BEGIN_BASE(Engine::Tools::SceneTool, Engine::Tools::ToolBase)
READONLY_PROPERTY(Views, views)
READONLY_PROPERTY(Files, files)
METATABLE_END(Engine::Tools::SceneTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::SceneTool, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::SceneTool)

namespace Engine {
namespace Tools {

    SceneTool::SceneTool(ImRoot &root)
        : Tool<SceneTool, ResourceEditor>(root)
        , SceneEditor(*this)
        , mWindow(static_cast<const ClientImRoot &>(root).window())
    {
    }

    Threading::Task<bool> SceneTool::init()
    {
        App::Application *app = App::Application::getSingletonPtr();
        if (!app)
            co_return false;
        mSceneMgr = &app->getGlobalAPIComponent<Scene::SceneManager>();

        mInspector = &mRoot.getTool<Inspector>();

        Render::SceneMainWindowComponent &main = mWindow.getWindowComponent<Render::SceneMainWindowComponent>();

        createView(main.renderData(), main.pointShadowRenderData(), Im3D::GetCurrentContext());

        co_return co_await ResourceEditor::init(Scene::SceneLoader::getSingleton(), "Scene");
    }

    Threading::Task<void> SceneTool::finalize()
    {
        clearViews();

        mFiles.clear();

        co_await ResourceEditor::finalize();
    }

    void SceneTool::render()
    {
        std::erase_if(mFiles, [&, this](std::pair<Scene::SceneLoader::Resource *const, SceneFile> &p) {
            return !p.second.render();
        });

        ResourceEditor::render();

        if (beginToolWindow("Scene", &mVisible, ImGuiWindowFlags_MenuBar)) {

            renderMenuBar();

            Behavior::BehaviorHandle behaviorToAdd = SceneEditor::render();

            if (behaviorToAdd) {
                mPendingBehavior.mTargetEntity = mSelectedEntity;
                mPendingBehavior.mHandle = behaviorToAdd;
                mPendingBehavior.mParameters = behaviorToAdd.createParameters();
                ImGui::OpenPopup("BehaviorParameters");
            }

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
                    Execution::access_binding(mPendingBehavior.mTargetEntity, [&](Scene::Entity::Entity &e) {
                        e.addBehavior(mPendingBehavior.mHandle.create(mPendingBehavior.mParameters));
                    });
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void SceneTool::renderMenu()
    {
        ResourceEditor::renderMenu();
    }

    void SceneTool::renderSettings()
    {
        ImGui::SeparatorText("Scene Editor");

        ImGui::Text("Bone-Forward");
        ImGui::SameLine();
        ImGui::DragFloat4("bone-forward", &mBoneForward.x);
        ImGui::DragFloat("Default Bone Length", &mDefaultBoneLength);
        ImGui::Checkbox("Show Bone Names", &mShowBoneNames);
        ImGui::Checkbox("Render 3D-Cursor", &mRender3DCursor);
    }

    std::string_view SceneTool::key() const
    {
        return "Scene";
    }

    void SceneTool::open(Resources::ResourceBase *res)
    {
        Scene::SceneLoader::Resource *scene = static_cast<Scene::SceneLoader::Resource *>(res);

        mFiles.try_emplace(scene, *this, scene);
    }

    Scene::SceneManager &SceneTool::sceneMgr()
    {
        return *mSceneMgr;
    }

    int SceneTool::createViewIndex()
    {
        return ++mRunningViewIndex;
    }

    void SceneTool::renderMenuBar()
    {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("SceneEditor")) {

                if (ImGui::MenuItem("Add View")) {
                    Render::SceneMainWindowComponent &main = mWindow.getWindowComponent<Render::SceneMainWindowComponent>();

                    createView(main.renderData(), main.pointShadowRenderData(), Im3D::GetCurrentContext());
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Panels")) {

                ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);
                ImGui::MenuItem("Entity Details", nullptr, &mEntityDetailsVisible);

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

}
}
