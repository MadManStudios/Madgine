#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Madgine/named.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

namespace Engine {
namespace Scene {

    struct EntityBinding : Named<"Entity", Entity::EntityPtr> {
        template <typename F>
        decltype(auto) sender(F &&f)
        {
            return Named<"Entity", Entity::EntityPtr>::sender([&](auto binding) {
                return (binding->*std::forward<F>(f))() | Execution::let_value([](auto &&sender) { return std::forward<decltype(sender)>(sender) | Execution::with_debug_location<Debug::SenderLocation>(); });
            });
        }
    };

    using NamedSceneManager = Named<"Scene", SceneManager *>;

    inline constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return scene.sender([=](SceneManager *mgr) { return mgr->simulationClock().wait(duration) | Execution::with_debug_location<Debug::SenderLocation>(); });
    };

    inline constexpr auto yield_simulation = [](NamedSceneManager scene = {}) {
        return wait_simulation(0s, scene);
    };
}
}
