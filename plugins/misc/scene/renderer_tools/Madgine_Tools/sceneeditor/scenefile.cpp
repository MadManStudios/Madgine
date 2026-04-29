#include "../scenerenderertoolslib.h"

#include "scenefile.h"

#include "Meta/serialize/formats.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine_Tools/behaviortool.h"
#include "Madgine_Tools/imgui/clientimroot.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "im3d/im3d.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "sceneeditor.h"
#include "scenetool.h"

METATABLE_BEGIN(Engine::Tools::SceneFile)
    READONLY_PROPERTY(Views, views)
    READONLY_PROPERTY(Scene, sceneMgr)
METATABLE_END(Engine::Tools::SceneFile)

namespace Engine {
namespace Tools {

    SceneFile::SceneFile(SceneTool &tool, Scene::SceneLoader::Resource *resource)
        : SceneEditor(tool)
        , ResourceFile(tool, resource ? resource->path() : "")
        , mManager(tool.sceneMgr().app(), std::nullopt)
        , mContainer(mManager.container("Editor"))
        , mSceneData(mManager)
        , mPointShadowRenderData(mManager, mSceneData)
        , mIm3DContext(Im3D::CreateContext())
    {
        mManager.startLifetime();

        if (resource) {
            resource->loadData(mContainer);
        }

        mPointShadowRenderData.setup(tool.mWindow.getRenderer());

        mSceneData.setup(tool.mWindow.getRenderer());

        mSceneViews.push_back(std::make_unique<SceneView>(*this, ++mRunningViewIndex, mSceneData, mPointShadowRenderData, mIm3DContext));
    }

    SceneFile::~SceneFile()
    {
        mSceneData.shutdown(tool().mWindow.getRenderer());

        mPointShadowRenderData.shutdown(tool().mWindow.getRenderer());

        mManager.endLifetime();

        Im3D::DestroyContext(mIm3DContext);
    }

    void SceneFile::saveAs(const Filesystem::Path &path)
    {
        Filesystem::FileManager mgr { "Scene" };

        Serialize::FormattedSerializeStream stream = mgr.openWrite(path, Serialize::Formats::xml);

        Serialize::write(stream, mContainer, "Container");

        mPath = path;

        mHistory.onSave();
    }

    void SceneFile::render()
    {
        Im3D::Im3DContext *context = Im3D::SetCurrentContext(mIm3DContext);

        Im3D::NewFrame();

        bool open = true;

        // mManager.simulationClock().tick(mManager.clock().now());

        if (Begin(&open)) {

            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Panels")) {

                    if (ImGui::MenuItem("Add View")) {
                        mSceneViews.push_back(std::make_unique<SceneView>(*this, ++mRunningViewIndex, mSceneData, mPointShadowRenderData, mIm3DContext));
                    }

                    ImGui::MenuItem("Hierarchy", nullptr, &tool().mHierarchyVisible);
                    ImGui::MenuItem("Widget Details", nullptr, &tool().mEntityDetailsVisible);

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            if (ImGui::BeginToolBar("Play")) {
                ImGui::SetNextItemShortcut(ImGuiKey_F5);
                if (ImGui::Button(IMGUI_ICON_PLAY)) {
                    tool().run(mContainer);
                }
                ImGui::EndToolBar();
            }

            Behavior::BehaviorHandle behaviorToAdd = SceneEditor::render(mHistory, mSceneViews);
            if (behaviorToAdd) {
                Execution::access_binding(mSelectedEntity, [&](Scene::Entity::Entity &entity) {
                    entity.behaviors().addBehavior(std::move(behaviorToAdd));
                });
            }
        }
        ImGui::End();

        Im3D::SetCurrentContext(context);

        if (!open) {
            if (mHistory.isDirty()) {
                mEditor.root().dialogs().showGrouped("Close", closeDialog(), [this]() { mCloseRequested = true; });
            } else {
                mCloseRequested = true;
            }
        }
    }

    Scene::SceneManager &SceneFile::sceneMgr()
    {
        return mManager;
    }

    std::vector<std::unique_ptr<SceneView>> &SceneFile::views()
    {
        return mSceneViews;
    }

}
}
