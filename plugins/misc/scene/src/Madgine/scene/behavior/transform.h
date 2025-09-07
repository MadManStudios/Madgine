#pragma once

#include "Generic/execution/algorithm.h"

#include "scenesenders.h"

#include "../scenemanager.h"

#include "../entity/components/transform.h"

#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Scene {

    constexpr auto rotate = [](Vector3 axis, float speed) {
        return yield_simulation() | Execution::let_value([=](std::chrono::microseconds timeSinceLastFrame) {
            return EntityBinding {} | Execution::let_value([=](auto &&entity) {
                return ((std::forward<decltype(entity)>(entity)->*[](Entity::Entity *e) { return e->getComponent<Entity::Transform>(); })()->*[=](Entity::Transform *transform) {
                    transform->mOrientation *= Quaternion { timeSinceLastFrame.count() * 0.000001f * speed, axis };
                })();
            });
        }) | Execution::repeat;
    };

}
}