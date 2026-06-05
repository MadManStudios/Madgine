#pragma once

#include "Meta/reflect/virtualscope.h"
#include "Meta/serialize/hierarchy/syncableunit.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Scene, SceneComponent, SceneComponentBase, Engine::Plugins::Constructor<SceneManager &>)

namespace Engine {
namespace Scene {

    template <typename T>
    using SceneComponent = Reflect::VirtualScope<T, Serialize::SyncableUnit<T, SceneComponentComponent<T>>>;

    template <typename T>
    using VirtualSceneComponentBase = Serialize::SyncableUnit<T, SceneComponentVirtualBase<T>>;

    template <typename T, typename Base>
    using VirtualSceneComponentImpl = Reflect::VirtualScope<T, Serialize::SyncableUnit<T, Plugins::VirtualComponentImpl<T, Base>>>;

#define VIRTUALSCENECOMPONENTBASE(T)                                                          \
    template <>                                                                               \
    TEMPLATE_INSTANCE constexpr size_t &Engine::Scene::VirtualSceneComponentBase<T>::sIndex() \
    {                                                                                         \
        static size_t index = -1;                                                             \
        return index;                                                                         \
    };

}
}
