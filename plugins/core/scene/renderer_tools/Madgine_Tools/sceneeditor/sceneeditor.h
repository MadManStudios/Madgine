#pragma once

#include "Madgine_Tools/toolscollector.h"

#include "Madgine_Tools/toolbase.h"

#include "sceneview.h"

#include "Madgine/scene/entity/entityptr.h"

#include "Madgine/scene/entity/entitycomponentptr.h"

#include "Madgine/parametertuple.h"

#include "Madgine/behaviorhandle.h"

namespace Engine {
namespace Tools {

    struct SceneEditor : Tool<SceneEditor> {

        SERIALIZABLEUNIT(SceneEditor)

        SceneEditor(ImRoot &root);
        SceneEditor(const SceneEditor &) = delete;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void render() override;
        virtual void renderMenu() override;
        virtual void renderSettings() override;

        std::string_view key() const override;

        const Filesystem::Path &currentSceneFile() const;

        std::vector<std::unique_ptr<SceneView>> &views()
        {
            return mSceneViews;
        }

        int hoveredAxis() const;
        const Engine::Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> &hoveredTransform() const;

        void deselect();
        void select(Render::Camera *camera);
        void select(const Scene::Entity::EntityPtr &entity);

        Scene::SceneManager &sceneMgr();

        void play();
        void pause();
        void stop();

        void openScene(const Filesystem::Path &p);
        void saveScene(const Filesystem::Path &p);

        int createViewIndex();

    private:
        void renderSelection();
        void renderHierarchy();
        void renderToolbar();
        void renderEntity(Scene::Entity::EntityPtr &entity);
        void renderCamera(Render::Camera *camera);

        void handleInputs();

        void im3DInteractions();

        void saveScenePopup();

    private:
        Window::MainWindow &mWindow;

        std::vector<std::unique_ptr<SceneView>> mSceneViews;

        Inspector *mInspector;
        Scene::SceneManager *mSceneMgr;

        Scene::Entity::EntityPtr mSelectedEntity;
        Render::Camera *mSelectedCamera = nullptr;

        enum { PLAY,
            STOP,
            PAUSE } mMode;

        //Save/Load
        std::vector<char> mStartBuffer;

        Filesystem::Path mCurrentSceneFile;

        //Entity-Cache
        struct EntityNode {
            Scene::Entity::EntityPtr mEntity;
            std::list<EntityNode> mChildren;
        };
        std::list<EntityNode> mEntityCache;
        std::map<Scene::Entity::EntityPtr, EntityNode *> mEntityMapping;

        void updateEntityCache();
        bool updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent = {});
        void createEntityMapping(Scene::Entity::EntityPtr e);
        void renderHierarchyEntity(EntityNode &entity);
        void eraseNode(EntityNode &node);

        int mHoveredAxis = -1;
        Engine::Scene::Entity::EntityComponentPtr<Scene::Entity::Transform> mHoveredTransform;

        struct {
            Scene::Entity::EntityPtr mTargetEntity;
            Threading::TaskFuture<ParameterTuple> mFuture;
            ParameterTuple mParameters;
            BehaviorHandle mHandle;
        } mPendingBehavior;

        //Settings
        Vector4 mBoneForward = { 1, 0, 0, 0 };
        float mDefaultBoneLength = 1.0f;
        bool mShowBoneNames = true;
        bool mRender3DCursor = false;

        int mRunningViewIndex = 0;
    };

}
}

REGISTER_TYPE(Engine::Tools::SceneEditor)