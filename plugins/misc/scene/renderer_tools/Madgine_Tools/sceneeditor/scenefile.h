#pragma once

#include "Madgine/render/pointshadowrenderdata.h"
#include "Madgine/render/scenerenderdata.h"
#include "Madgine/scene/sceneloader.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine_Tools/resourcefile.h"
#include "entitycache.h"
#include "sceneeditor.h"

namespace Engine {

namespace Im3D {
    struct Im3DContext;
}

namespace Tools {

    struct SceneFile : ResourceFile<SceneTool>, SceneEditor {

        SceneFile(SceneTool &tool, Scene::SceneLoader::Resource *resource);
        ~SceneFile();

        void saveAs(const Filesystem::Path &path) override;

        void render();

        Scene::SceneManager &sceneMgr() override;

    protected:
        void renderEntity(Scene::Entity::EntityPtr &entity);
        void renderHierarchyEntity(const EntityCache::Node &entity);

    private:

        Scene::SceneManager mManager;
        Scene::SceneContainer &mContainer;

        Render::SceneRenderData mSceneData;

        Render::PointShadowRenderData mPointShadowRenderData;

        Im3D::Im3DContext *mIm3DContext;
    };

}
}