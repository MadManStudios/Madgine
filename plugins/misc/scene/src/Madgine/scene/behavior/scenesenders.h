#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"

#include "Madgine/behavior/named.h"

#include "../entity/components/transform.h"
#include "../scenemanager.h"

namespace Engine {
namespace Scene {

    using EntityBinding = Behavior::Named<"Entity", Entity::EntityPtr>;

    using NamedSceneManager = Behavior::Named<"Scene", SceneManager &>;

    inline constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return std::move(scene) | Execution::let_value([=](SceneManager &mgr) { return mgr.simulationClock().wait(duration); });
    };

    inline constexpr auto yield_simulation = [](NamedSceneManager scene = {}) {
        return wait_simulation(0s, scene);
    };

    inline constexpr auto wait_animation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return std::move(scene) | Execution::let_value([=](SceneManager &mgr) { return mgr.animationClock().wait(duration); });
    };

    inline constexpr auto yield_animation = [](NamedSceneManager scene = {}) {
        return wait_animation(0s, scene);
    };
}
}
