#pragma once

#include "Madgine_Tools/toolscollector.h"

#include "Madgine_Tools/toolbase.h"

#include "sceneview.h"

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
        Scene::Entity::Transform *const &hoveredTransform() const;

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
        void renderDetails();
        void renderHierarchy();
        void renderEntity(Scene::Entity::EntityPtr &entity);
        void renderCamera(Render::Camera *camera);

        void handleInputs();

        void im3DInteractions();

        void saveScenePopup();

    private:
        Window::MainWindow &mWindow;

        std::vector<std::unique_ptr<SceneView>> mSceneViews;

        bool mHierarchyVisible = true;
        bool mEntityDetailsVisible = true;

        Inspector *mInspector;
        Scene::SceneManager *mSceneMgr;

        Scene::Entity::EntityPtr mSelectedEntity;
        Render::Camera *mSelectedCamera = nullptr;

        enum { PLAY,
            STOP,
            PAUSE } mMode;

        // Save/Load
        std::vector<char> mStartBuffer;

        Filesystem::Path mCurrentSceneFile;

        // Entity-Cache
        struct EntityNode {
            Scene::Entity::EntityPtr mEntity;
            std::list<EntityNode> mChildren;
        };
        struct EntityComparator {
            bool operator()(const Scene::Entity::EntityPtr &a, const Scene::Entity::EntityPtr &b) const
            {
                const auto helper = [](Scene::Entity::Entity &entity) { return &entity; };
                return (a->*helper)() < (b->*helper)();
            }
        };

        std::list<EntityNode> mEntityCache;
        std::map<Scene::Entity::EntityPtr, EntityNode *, EntityComparator> mEntityMapping;

        void updateEntityCache();
        bool updateEntityCache(EntityNode &node, const Scene::Entity::EntityPtr &parent = {});
        void createEntityMapping(Scene::Entity::EntityPtr e);
        void renderHierarchyEntity(EntityNode &entity);
        void eraseNode(EntityNode &node);

        int mHoveredAxis = -1;
        Scene::Entity::Transform *mHoveredTransform;

        struct {
            Scene::Entity::EntityPtr mTargetEntity;
            Threading::TaskFuture<ParameterTuple> mFuture;
            ParameterTuple mParameters;
            BehaviorHandle mHandle;
        } mPendingBehavior;

        // Settings
        Vector4 mBoneForward = { 1, 0, 0, 0 };
        float mDefaultBoneLength = 1.0f;
        bool mShowBoneNames = true;
        bool mRender3DCursor = false;

        int mRunningViewIndex = 0;
    };

}
}