#pragma once

#include "Madgine/behavior/behaviorhandle.h"
#include "Madgine/behavior/parametertuple.h"
#include "Madgine/scene/sceneloader.h"

#include "Madgine_Tools/resources/resourceeditor.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "entitycache.h"
#include "sceneeditor.h"
#include "scenefile.h"
#include "sceneview.h"

namespace Engine {
namespace Tools {

    struct SceneTool : Tool<SceneTool, ResourceEditor>, SceneEditor {

        SERIALIZABLEUNIT(SceneTool)

        SceneTool(ImRoot &root);
        SceneTool(const SceneTool &) = delete;

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void update() override;
        void render() override;
        void renderMenu() override;
        void renderSettings() override;

        std::string_view key() const override;

        void open(Resources::ResourceBase *res) override;

        Scene::SceneManager &sceneMgr() override;

        auto files()
        {
            return mFiles | std::views::transform([](std::pair<Scene::SceneLoader::Resource *const, SceneFile> &file) {
                return std::make_pair(file.first->name(), &file.second);
            });
        }

        Dialog<> closeDialog() override;

    private:
        void renderMenuBar();

    private:
        Window::MainWindow &mWindow;

        friend struct SceneEditor;
        friend struct SceneFile;
        friend struct SceneView;
        std::map<Scene::SceneLoader::Resource *, SceneFile> mFiles;

        bool mHierarchyVisible = true;
        bool mEntityDetailsVisible = true;

        Inspector *mInspector;
        Scene::SceneManager *mSceneMgr;

        struct {
            Scene::Entity::EntityPtr mTargetEntity;
            Behavior::ParameterTuple mParameters;
            Behavior::BehaviorHandle mHandle;
        } mPendingBehavior;

        UndoStack mHistory;

        // Settings
        Vector4 mBoneForward = { 1, 0, 0, 0 };
        float mDefaultBoneLength = 1.0f;
        bool mShowBoneNames = true;
        bool mRender3DCursor = false;
    };

}
}