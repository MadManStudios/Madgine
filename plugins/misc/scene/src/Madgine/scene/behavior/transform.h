#pragma once

#include "Generic/execution/algorithm.h"

#include "../entity/components/transform.h"
#include "scenesenders.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Math::Vector3 axis, float speed, EntityContext entity = {}) {
        return std::move(entity) | Execution::let_value([=](auto e) {
            return yield_simulation() | Execution::then([=](std::chrono::microseconds timeSinceLastFrame) {
                Execution::access_binding(e, [=](Entity::Entity &e) {
                    e.getComponent<Entity::Transform>()->mOrientation *= Math::Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
                });
            }) | Execution::repeat;
        });
    };

    constexpr auto translate = [](Math::NormalizedVector3 dir, float speed, EntityContext entity = {}) {
        return std::move(entity) | Execution::let_value([=](auto e) {
            return yield_simulation() | Execution::then([=](std::chrono::microseconds timeSinceLastFrame) {
                Execution::access_binding(e, [=](Entity::Entity &e) {
                    e.getComponent<Entity::Transform>()->mPosition += timeSinceLastFrame.count() * 0.000001f * speed * dir;
                });
            }) | Execution::repeat;
        });
    };

}
}