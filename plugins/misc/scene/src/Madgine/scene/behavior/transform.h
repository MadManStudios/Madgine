#pragma once

#include "Generic/execution/algorithm.h"

#include "scenesenders.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Vector3 axis, float speed) {
        return EntityBinding {}.sender([=](auto &&binding) {
            return yield_simulation() | Execution::let_value([=](std::chrono::microseconds timeSinceLastFrame) {
                return (binding->*[=](Entity::Entity *e) { 
                    e->getComponent<Entity::Transform>()->mOrientation *= Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
                })();
            }) | Execution::repeat;
        });
    };

}
}