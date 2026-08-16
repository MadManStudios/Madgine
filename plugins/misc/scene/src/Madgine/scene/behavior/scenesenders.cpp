#include "../../scenelib.h"

#include "scenesenders.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/nativebehaviorcollector.h"

#include "../entity/components/transform.h"

NATIVE_BEHAVIOR(Yield_Simulation, Engine::Scene::yield_simulation)
NATIVE_BEHAVIOR(Wait_Simulation, Engine::Scene::wait_simulation, Engine::Behavior::InputParameter<"Duration", std::chrono::steady_clock::duration>)

namespace Engine {
namespace Scene {

    void lookAt(const Entity::EntityPtr &e, const Entity::EntityPtr &target, bool locked, std::chrono::steady_clock::duration delta)
    {
        Execution::access_binding(target, [&](Entity::Entity &target) {
            return Execution::access_binding(e, [&](Entity::Entity &entity) {
                Entity::Transform *targetTransform = target.getComponent<Entity::Transform>();
                if (!targetTransform)
                    return false;
                Entity::Transform *entityTransform = entity.getComponent<Entity::Transform>();
                if (!entityTransform)
                    return false;

                Math::Quaternion targetOrientation;
                if (locked)
                    targetOrientation = Math::Quaternion::FromDirectionLockedUp(targetTransform->worldPosition(target) - entityTransform->worldPosition(entity));
                else
                    targetOrientation = Math::Quaternion::FromDirection(targetTransform->worldPosition(target) - entityTransform->worldPosition(entity));

                entityTransform->setWorldOrientation(targetOrientation, entity);

                return true;
            });
        });
    }

}
}

NATIVE_BEHAVIOR(Look_At, Engine::Scene::look_at, Engine::Behavior::InputParameter<"Target", Engine::Scene::Entity::EntityPtr>, Engine::Behavior::InputParameter<"Locked", bool>)
