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
            return Named<"Entity", Entity::EntityPtr>::sender([f { forward_capture<F>(f) }](auto binding) mutable {
                return (binding->*std::forward<F>(f))() | Execution::let_value([](auto &&sender) -> decltype(auto) { return std::forward<decltype(sender)>(sender); });
            });
        }
    };

    using NamedSceneManager = Named<"Scene", SceneManager &>;

    inline constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return scene.sender([=](SceneManager &mgr) { return mgr.simulationClock().wait(duration); });
    };

    inline constexpr auto yield_simulation = [](NamedSceneManager scene = {}) {
        return wait_simulation(0s, scene);
    };
}
}
