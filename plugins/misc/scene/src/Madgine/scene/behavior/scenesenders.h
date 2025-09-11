#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Madgine/named.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    struct EntityBinding : Named<"Entity", Execution::BindingPtr<Entity::Entity *>> {
        template <typename F>
        decltype(auto) sender(F &&f)
        {
            return Named<"Entity", Execution::BindingPtr<Entity::Entity *>>::sender([&](auto binding) {
                return (binding->*std::forward<F>(f))();
            });
        }
    };

    using NamedSceneManager = Named<"Scene", SceneManager *>;

    inline constexpr auto wait_simulation = [](std::chrono::steady_clock::duration duration, NamedSceneManager scene = {}) {
        return scene.sender([=](SceneManager *mgr) { return mgr->simulationClock().wait(duration); });
    };

    inline constexpr auto yield_simulation = [](NamedSceneManager scene = {}) {
        return wait_simulation(0s, scene);
    };
}
}
