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

    struct MADGINE_SCENE_RENDERER_EXPORT SceneMainWindowComponent : Window::MainWindowComponent<SceneMainWindowComponent> {

        SERIALIZABLEUNIT(SceneMainWindowComponent)

        SceneMainWindowComponent(Window::MainWindow &window);
        ~SceneMainWindowComponent();

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;

        Scene::SceneManager &scene();

        SceneRenderData &renderData();
        PointShadowRenderData &pointShadowRenderData();

        void enableSceneRendering();
        void disableSceneRendering();

        Camera mCamera;

    private:
        Scene::SceneManager &mScene;

        SceneRenderData mSceneData;

        PointShadowRenderData mPointShadowRenderData;

        SceneRenderPass mPass;
    };

}
}