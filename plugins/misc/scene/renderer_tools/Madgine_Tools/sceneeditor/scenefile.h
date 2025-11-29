#pragma once

#include "Madgine/scene/sceneloader.h"

#include "Madgine/scene/scenemanager.h"

#include "entitycache.h"

#include "Madgine/render/pointshadowrenderdata.h"

#include "Madgine/render/scenerenderdata.h"

#include "sceneeditor.h"

namespace Engine {

namespace Im3D {
    struct Im3DContext;
}

namespace Tools {

    struct SceneFile : SceneEditor {

        SceneFile(SceneTool &tool, Scene::SceneLoader::Resource *resource);
        ~SceneFile();

        void save(const Filesystem::Path &path);

        bool render();

        Scene::SceneManager &sceneMgr() override;

    protected:
        void renderEntity(Scene::Entity::EntityPtr &entity);
        void renderHierarchyEntity(const EntityCache::Node &entity);

    private:
        Filesystem::Path mPath;
        bool mIsDirty = true;

        Scene::SceneManager mManager;
        Scene::SceneContainer &mContainer;
                
        Render::SceneRenderData mSceneData;

        Render::PointShadowRenderData mPointShadowRenderData;

        Im3D::Im3DContext *mIm3DContext;
    };

}
}