#pragma once

#include "Generic/execution/algorithm.h"
#include "Madgine/bindings.h"

#include "scenesenders.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Vector3 axis, float speed) {
        return yield_simulation() | Execution::let_value([=](std::chrono::microseconds timeSinceLastFrame) {
            return get_component<Entity::Transform>() | Execution::then([=](Entity::Transform *transform) {
                transform->mOrientation *= Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
            });
        }) | Execution::repeat;
    };

}
}

NATIVE_BEHAVIOR_DECLARATION(Rotate)