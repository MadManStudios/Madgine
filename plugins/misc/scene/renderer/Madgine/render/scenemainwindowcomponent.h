#pragma once

#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "scenerenderpass.h"

#include "Madgine/render/camera.h"

#include "Meta/math/color3.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT SceneMainWindowComponent : Window::MainWindowComponent<SceneMainWindowComponent> {

        SERIALIZABLEUNIT(SceneMainWindowComponent)

        SceneMainWindowComponent(Window::MainWindow &window);
        ~SceneMainWindowComponent();

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;

        Scene::SceneManager *scene();

        std::vector<const Texture*> depthTextures();

        Render::RenderTarget *pointShadowTarget(size_t index);
        Render::RenderData *data();

        void enableSceneRendering();
        void disableSceneRendering();

        Camera mCamera;

        Color3 mAmbientLightColor = { 1.0f, 1.0f, 1.0f };
        NormalizedVector3 mAmbientLightDirection = { -0.0f, -1.0f, 1.5f };

    private:
        Scene::SceneManager *mScene = nullptr;

        SceneRenderPass mPass;

        std::unique_ptr<Render::RenderTarget> mPointShadowMaps[2];

        std::unique_ptr<SceneRenderData> mSceneData;

        PointShadowRenderPass mPointShadowPasses[2];
    };

}
}

REGISTER_TYPE(Engine::Render::SceneMainWindowComponent)