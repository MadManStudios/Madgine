#include "../scenelib.h"
#include "scenecomponentbase.h"
#include "scenemanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Scene::SceneComponentBase)
METATABLE_END(Engine::Scene::SceneComponentBase)

SERIALIZETABLE_BEGIN(Engine::Scene::SceneComponentBase)
SERIALIZETABLE_END(Engine::Scene::SceneComponentBase)

namespace Engine {

namespace Scene {
    SceneComponentBase::SceneComponentBase(SceneManager &sceneMgr)
        : mSceneMgr(sceneMgr)
    {
    }

    Threading::Task<bool> SceneComponentBase::init()
    {
        co_return true;
    }

    Threading::Task<void> SceneComponentBase::finalize()
    {
        co_return;
    }

    SceneManager &SceneComponentBase::sceneMgr() const
    {
        return mSceneMgr;
    }

    std::string_view SceneComponentBase::key() const
    {
        return serializeType()->mTypeName;
    }

    SceneComponentBase &SceneComponentBase::getSceneComponent(size_t i)
    {
        return mSceneMgr.getComponent(i);
    }

    App::GlobalAPIBase &SceneComponentBase::getGlobalAPIComponent(size_t i)
    {
        return mSceneMgr.getGlobalAPIComponent(i);
    }

    Threading::TaskQueue *SceneComponentBase::taskQueue() const
    {
        return mSceneMgr.taskQueue();
    }

    void SceneComponentBase::updateFrame(std::chrono::microseconds frameTimeSinceLastFrame, std::chrono::microseconds sceneTimeSinceLastFrame) {

    }

}
}
