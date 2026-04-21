#include "../scenerendererlib.h"

#include "scenemainwindowcomponent.h"

#include "Meta/math/matrix4.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/app/application.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/scene/scenemanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "scenerenderdata.h"

NAMED_UNIQUECOMPONENT(SceneMainWindowComponent, Engine::Render::SceneMainWindowComponent)

METATABLE_BEGIN(Engine::Render::SceneMainWindowComponent)
    MEMBER(mCamera)
    PROPERTY(RenderingEnabled, renderingEnabled, setRenderingEnabled)
METATABLE_END(Engine::Render::SceneMainWindowComponent)

SERIALIZETABLE_BEGIN(Engine::Render::SceneMainWindowComponent)
    ENCAPSULATED_FIELD(RenderingEnabled, renderingEnabled, setRenderingEnabled)
SERIALIZETABLE_END(Engine::Render::SceneMainWindowComponent)

namespace Engine {
namespace Render {

    SceneMainWindowComponent::SceneMainWindowComponent(Window::MainWindow &window)
        : MainWindowComponent(window, 5)
        , mScene(window.app().getGlobalAPIComponent<Engine::Scene::SceneManager>())
        , mSceneData(mScene)
        , mPointShadowRenderData(mScene, mSceneData)
        , mPass(mScene, mSceneData, mPointShadowRenderData, mCamera, 1)
    {
    }

    SceneMainWindowComponent::~SceneMainWindowComponent() = default;

    void SceneMainWindowComponent::render(RenderTarget *target, size_t iteration)
    {
        MainWindowComponentBase::render(target, iteration);

        if (mRenderingEnabled) {
            mPass.render(target, iteration);
        }
    }

    void SceneMainWindowComponent::setup(RenderTarget *target)
    {
        mPointShadowRenderData.setup(target->context());

        mSceneData.setup(target->context());

        mPass.setup(target);
    }

    void SceneMainWindowComponent::shutdown(RenderTarget *target)
    {
        mPass.shutdown(target);

        mPointShadowRenderData.shutdown(target->context());

        mSceneData.shutdown(target->context());
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

    void SceneMainWindowComponent::setRenderingEnabled(bool enabled)
    {
        if (mRenderingEnabled != enabled) {
            mRenderingEnabled = enabled;

            if (mRenderingEnabled) {
                for (RenderData *dependency : mPass.dependencies()) {
                    addDependency(dependency);
                }
            } else {
                for (RenderData *dependency : mPass.dependencies()) {
                    removeDependency(dependency);
                }
            }
        }
    }

    bool SceneMainWindowComponent::renderingEnabled() const
    {
        return mRenderingEnabled;
    }

}
}