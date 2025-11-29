#include "../scenerendererlib.h"

#include "scenemainwindowcomponent.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/app/application.h"
#include "Madgine/scene/scenemanager.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "Meta/math/matrix4.h"

#include "scenerenderdata.h"

NAMED_UNIQUECOMPONENT(SceneMainWindowComponent, Engine::Render::SceneMainWindowComponent)

METATABLE_BEGIN(Engine::Render::SceneMainWindowComponent)
MEMBER(mCamera)
METATABLE_END(Engine::Render::SceneMainWindowComponent)

SERIALIZETABLE_BEGIN(Engine::Render::SceneMainWindowComponent)
SERIALIZETABLE_END(Engine::Render::SceneMainWindowComponent)

namespace Engine {
namespace Render {

    SceneMainWindowComponent::SceneMainWindowComponent(Window::MainWindow &window)
        : MainWindowComponent(window, 5)
        , mScene(Engine::App::Application::getSingleton().getGlobalAPIComponent<Engine::Scene::SceneManager>())
        , mSceneData(mScene)
        , mPointShadowRenderData(mScene, mSceneData)
        , mPass(mScene, mSceneData, mPointShadowRenderData, mCamera, 1)
    {
    }

    Engine::Render::SceneMainWindowComponent::~SceneMainWindowComponent() = default;

    void SceneMainWindowComponent::setup(RenderTarget *target)
    {
        mPointShadowRenderData.setup(target->context());
    }

    void SceneMainWindowComponent::shutdown(RenderTarget *target)
    {
        mPointShadowRenderData.shutdown(target->context());
    }

    Scene::SceneManager &SceneMainWindowComponent::scene()
    {
        return mScene;
    }

    SceneRenderData &SceneMainWindowComponent::renderData()
    {
        return mSceneData;
    }

    PointShadowRenderData &SceneMainWindowComponent::pointShadowRenderData()
    {
        return mPointShadowRenderData;
    }

    void SceneMainWindowComponent::enableSceneRendering()
    {
        mWindow.getRenderWindow()->addRenderPass(&mPass);
    }

    void Engine::Render::SceneMainWindowComponent::disableSceneRendering()
    {
        mWindow.getRenderWindow()->removeRenderPass(&mPass);
    }

}
}