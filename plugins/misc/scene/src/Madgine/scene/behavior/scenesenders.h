#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"

#include "Madgine/behavior/context.h"

#include "../scenemanager.h"

namespace Engine {
namespace Scene {

    using EntityBinding = Behavior::ContextParameter<Entity::EntityPtr>;

    using NamedSceneManager = Behavior::ContextParameter<SceneManager &>;

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

    using yield_animation_t = decltype(yield_animation());

    void lookAt(const Entity::EntityPtr &e, const Entity::EntityPtr &target, bool locked, std::chrono::steady_clock::duration delta);

    inline constexpr auto look_at = [](Entity::EntityPtr target, bool locked = false, EntityBinding entity = {}) {
        return std::move(entity) | Execution::let_value([=](Entity::EntityPtr e) { return yield_animation() | Execution::then([=](std::chrono::steady_clock::duration delta) { lookAt(e, target, locked, delta); }) | Execution::repeat; });
    };
    
}
}
