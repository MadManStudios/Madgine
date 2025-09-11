#pragma once

#include "Generic/execution/algorithm.h"

#include "scenesenders.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Vector3 axis, float speed) {
        return EntityBinding {}.sender([=](Entity::Entity *e) {
            return yield_simulation() | Execution::then([=](std::chrono::microseconds timeSinceLastFrame) {
                e->getComponent<Entity::Transform>()->mOrientation *= Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
            }) | Execution::repeat;
        });
    };

}
}