#pragma once

#include "Madgine/behavior/behaviorhandle.h"
#include "Madgine/behavior/parametertuple.h"
#include "Madgine/scene/sceneloader.h"

#include "Madgine_Tools/resourceeditor.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "entitycache.h"
#include "sceneview.h"

namespace Engine {
namespace Tools {

    struct SceneEditor {

        SceneEditor(SceneTool &tool);

        std::vector<std::unique_ptr<SceneView>> &views()
        {
            return mSceneViews;
        }

        int hoveredAxis() const;
        Scene::Entity::Transform *const &hoveredTransform() const;

        void deselect();
        void select(const Scene::Entity::EntityPtr &entity);

        virtual Scene::SceneManager &sceneMgr() = 0;

        SceneTool &tool();

        std::string patchIcon(std::string_view label);

        void createView(Render::SceneRenderData &sceneData, Render::PointShadowRenderData &pointShadowRenderData, Im3D::Im3DContext *context);
        void clearViews();

        Behavior::BehaviorHandle render();

    private:
        void renderToolBar();

        Behavior::BehaviorHandle renderDetails();
        void renderHierarchy();
        Behavior::BehaviorHandle renderEntity(Scene::Entity::EntityPtr &entity);
        void renderHierarchyEntity(const EntityCache::Node &entity, bool visible);

        void handleInputs();

        void im3DInteractions();

    protected:
        Scene::Entity::EntityPtr mSelectedEntity;

    private:
        std::vector<std::unique_ptr<SceneView>> mSceneViews;

        EntityCache mEntityCache;

        SceneTool &mTool;

        int mHoveredAxis = -1;
        Scene::Entity::Transform *mHoveredTransform;
    };

}
}