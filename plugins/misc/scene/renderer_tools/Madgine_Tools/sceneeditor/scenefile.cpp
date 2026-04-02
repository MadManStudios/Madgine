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
        , mPath(resource ? resource->path() : "")
        , mManager(tool.sceneMgr().app())
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

        createView(mSceneData, mPointShadowRenderData, mIm3DContext);
    }

    SceneFile::~SceneFile()
    {
        mSceneData.shutdown(tool().mWindow.getRenderer());

        mPointShadowRenderData.shutdown(tool().mWindow.getRenderer());

        mManager.endLifetime();

        Im3D::DestroyContext(mIm3DContext);
    }

    void SceneFile::save(const Filesystem::Path &path)
    {
        Filesystem::FileManager mgr { "Scene" };

        Serialize::FormattedSerializeStream stream = mgr.openWrite(path, Serialize::Formats::xml);

        Serialize::write(stream, mContainer, "Container");

        mPath = path;
        mIsDirty = false;
    }

    bool SceneFile::render()
    {
        Im3D::Im3DContext *context = Im3D::SetCurrentContext(mIm3DContext);

        Im3D::NewFrame();

        bool open = true;

        //mManager.simulationClock().tick(mManager.clock().now());

        if (tool().BeginResourceFile(this, mPath, mIsDirty, [this](const Filesystem::Path &path) { save(path); }, &open)) {

            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Panels")) {

                    ImGui::MenuItem("Hierarchy", nullptr, &tool().mHierarchyVisible);
                    ImGui::MenuItem("Widget Details", nullptr, &tool().mEntityDetailsVisible);

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            Behavior::BehaviorHandle behaviorToAdd = SceneEditor::render();
            if (behaviorToAdd) {
                Execution::access_binding(mSelectedEntity, [&](Scene::Entity::Entity &entity) {
                    entity.behaviors().addBehavior(std::move(behaviorToAdd));
                });
            }
        }
        ImGui::End();

        Im3D::SetCurrentContext(context);

        return open;
    }

    Scene::SceneManager &SceneFile::sceneMgr()
    {
        return mManager;
    }

}
}
