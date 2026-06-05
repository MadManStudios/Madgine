#pragma once

#include "Meta/reflect/virtualscope.h"
#include "Meta/serialize/hierarchy/syncableunit.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Scene {

    struct MADGINE_SCENE_EXPORT SceneComponentBase : Reflect::VirtualScopeBase<>, Serialize::SyncableUnitBase, Threading::MadgineObject<SceneComponentBase> {
        virtual ~SceneComponentBase() = default;

        SceneComponentBase(SceneManager &sceneMgr);

        SceneManager &sceneMgr() const;

        virtual void updateFrame(std::chrono::microseconds frameTimeSinceLastFrame, std::chrono::microseconds sceneTimeSinceLastFrame);
        std::string_view key() const;

        template <typename T>
        T &getSceneComponent()
        {
            return static_cast<T &>(getSceneComponent(component_index<T>()));
        }

        SceneComponentBase &getSceneComponent(size_t i);

        template <typename T>
        T &getGlobalAPIComponent()
        {
            return static_cast<T &>(getGlobalAPIComponent(component_index<T>()));
        }

        Core::GlobalAPIBase &getGlobalAPIComponent(size_t i);

        Threading::TaskQueue *taskQueue() const;

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<SceneComponentBase>;

        SceneManager &mSceneMgr;
    };

}
}