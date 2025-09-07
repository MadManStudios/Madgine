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
    constexpr EntityBinding entityBinding;
    
    using SceneBinding = Named<"Scene", SceneManager *>;
    constexpr SceneBinding sceneBinding;

    inline constexpr auto wait_simulation = []<typename Binding = const SceneBinding &>(std::chrono::steady_clock::duration duration, Binding &&scene = sceneBinding) {
        return std::forward<Binding>(scene) | Execution::let_value([=](SceneManager *scene) { return scene->simulationClock().wait(duration); });
    };

    inline constexpr auto yield_simulation = []<typename Binding = const SceneBinding &>(Binding &&scene = sceneBinding) {
        return wait_simulation(0s, std::forward<Binding>(scene));
    };

}
}
