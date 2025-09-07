#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Madgine/named.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    using EntityBinding = Named<"Entity", Execution::BindingPtr<Entity::Entity *>>;
    using NamedSceneManager = Named<"Scene", SceneManager *>;

    inline constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return scene.sender([=](SceneManager *mgr) { return mgr->simulationClock().wait(duration); });
    };

    inline constexpr auto yield_simulation = [](NamedSceneManager scene = {}) {
        return wait_simulation(0s, scene);
    };

}
}
