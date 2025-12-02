#pragma once

#include "Generic/execution/algorithm.h"

#include "scenesenders.h"

#include "../entity/components/transform.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Vector3 axis, float speed, EntityBinding entity = {}) {
        return entity | Execution::let_value([=](auto e) {
            return yield_animation() | Execution::then([=](std::chrono::microseconds timeSinceLastFrame) {
                Execution::access_binding(e, [=](Entity::Entity &e) {
                    e.getComponent<Entity::Transform>()->mOrientation *= Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
                });
            }) | Execution::repeat;
        });
    };

}
}