#pragma once

#include "Generic/execution/algorithm.h"
#include "Madgine/bindings.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"


#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    using EntityBinding = Binding<"Entity", Entity::Entity*>;
    constexpr EntityBinding entityBinding;

    using SceneBinding = Binding<"Scene", SceneManager*>;
    constexpr SceneBinding sceneBinding;

    constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, SceneBinding entity = {}) {
        return IntervalClock<Threading::CustomTimepoint>::wait(entity | Execution::then(&SceneManager::simulationClock), duration);
    };

    constexpr auto yield_simulation = [](SceneBinding scene = {}) {
        return wait_simulation(0s, scene);
    };

    template <typename T>
    constexpr auto get_component = [](EntityBinding entity = {}) { return entity | Execution::then([](Entity::Entity *e) { return e->getComponent<T>(); }); };

}
}

NATIVE_BEHAVIOR_DECLARATION(Yield_Simulation)
NATIVE_BEHAVIOR_DECLARATION(Wait_Simulation)
