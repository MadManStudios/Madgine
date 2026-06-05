#pragma once

#include "Meta/math/color3.h"

#include "Madgine/render/camera.h"
#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "pointshadowrenderdata.h"
#include "scenerenderdata.h"
#include "scenerenderpass.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT SceneMainWindowComponent : Core::MainWindowComponent<SceneMainWindowComponent> {

        SERIALIZABLEUNIT(SceneMainWindowComponent)

        SceneMainWindowComponent(Core::MainWindow &window);
        ~SceneMainWindowComponent();

        void render(RenderTarget *target, size_t iteration) override;
        void setup(RenderTarget *target) override;
        void shutdown(RenderTarget *target) override;

        Scene::SceneManager &scene();

        SceneRenderData &renderData();
        PointShadowRenderData &pointShadowRenderData();

        void setRenderingEnabled(bool enabled);
        bool renderingEnabled() const;

        Camera mCamera;

    private:
        Scene::SceneManager &mScene;

        SceneRenderData mSceneData;

        PointShadowRenderData mPointShadowRenderData;

        SceneRenderPass mPass;

        bool mRenderingEnabled = false;
    };

}
}