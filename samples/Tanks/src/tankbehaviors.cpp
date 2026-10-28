#include "Madgine/audiolib.h"
#include "Madgine/bulletlib.h"
#include "Madgine/scenelib.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/audio/audioloader.h"
#include "Madgine/audio/senders/audiosenders.h"
#include "Madgine/audio/sources/effects.h"
#include "Madgine/behavior/awaitables/awaitablebinding.h"
#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/nativebehaviorcollector.h"
#include "Madgine/physics/rigidbody.h"
#include "Madgine/scene/behavior/scenesenders.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entitydescriptor.h"
#include "Madgine/scene/entity/entityptr.h"

#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

namespace Engine {

Behavior::Behavior TankMovement(Scene::EntityContext tank = {})
{
    std::chrono::steady_clock::duration acc = 5s;

    float pan = 0.0f;
    float velocity = 0.0f;

    auto tankEntity = co_await *tank;
    Scene::Entity::Transform *transform = tankEntity->getComponent<Scene::Entity::Transform>();
    Physics::RigidBody *rigidBody = tankEntity->getComponent<Physics::RigidBody>();

    while (true) {
        acc += co_await Scene::yield_simulation();

        if (acc >= 5s) {
            acc -= 5s;

            pan = sqrt(std::rand() / float(RAND_MAX)) * 6.0f;
            velocity = (std::rand() % 2 ? 1.0f : -1.0f) * sqrt(std::rand() / float(RAND_MAX)) * 200.f;
        }

        Math::Vector3 vel = velocity * (transform->mOrientation * Math::Vector3::UNIT_Z);

        assert(!isnan(vel.x) && !isnan(vel.y) && !isnan(vel.z));

        rigidBody->get()->applyCentralForce({ vel.x, vel.y, vel.z });

        rigidBody->get()->applyTorque({ 0.0f, pan, 0.0f });

        rigidBody->activate();
    }

    co_return;
}

Behavior::Behavior AudioTest(Scene::EntityContext self = {})
{
    auto selfPtr = co_await *self;

    co_await Audio::play_source(Audio::AudioLoader::Handle { Audio::AudioLoader::load("sample") } | Audio::looping | Audio::positional(Execution::FunctionBinding { [](Scene::Entity::Entity &e) { return Math::Vector2 { e.getComponent<Scene::Entity::Transform>()->mPosition.x, e.getComponent<Scene::Entity::Transform>()->mPosition.z }; } }(*self)) | Audio::volume(0.1f), "Default");
}

}

NATIVE_BEHAVIOR(TankMovement, Engine::TankMovement)
NATIVE_BEHAVIOR(AudioTest, Engine::AudioTest)
