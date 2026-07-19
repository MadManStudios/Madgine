#pragma once

#include "Madgine/behavior/behaviorhandle.h"
#include "Madgine/scene/sceneloader.h"

#include "Madgine_Tools/resources/resourceeditor.h"
#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"
#include "entitycache.h"
#include "sceneview.h"

namespace Engine {
namespace Tools {

    struct SceneEditor {

        SceneEditor(SceneTool &tool);

        IndexType<uint8_t> hoveredAxis() const;
        const Scene::Entity::EntityPtr &hoveredEntity() const;

        void deselect();
        void select(const Scene::Entity::EntityPtr &entity);

        virtual Scene::SceneManager &sceneMgr() = 0;

        SceneTool &tool();

        std::string patchIcon(std::string_view label);

        Behavior::BehaviorHandle render(UndoStack &history, std::vector<std::unique_ptr<SceneView>> &views);

    private:
        void renderToolBar();

        Behavior::BehaviorHandle renderDetails(UndoStack &history);
        void renderHierarchy(UndoStack &history);
        Behavior::BehaviorHandle renderEntity(Scene::Entity::EntityPtr &entity, UndoStack &history);
        void renderHierarchyEntity(const EntityCache::Node &entity, bool visible, UndoStack &history);

        void handleInputs();

        void im3DInteractions();

    protected:
        Scene::Entity::EntityPtr mSelectedEntity;

        EntityCache mEntityCache;

        int mRunningViewIndex = 0;

    private:
        SceneTool &mTool;

        IndexType<uint8_t> mHoveredAxis;
        Scene::Entity::EntityPtr mHoveredEntity; 
        
    };

}
}