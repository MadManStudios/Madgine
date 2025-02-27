#pragma once

#include "Madgine/scene/entity/entitycomponent.h"

#include "Madgine/scene/entity/entitycomponentptr.h"

#include "collisionshapemanager.h"

class btRigidBody;

namespace Engine {
namespace Physics {

    struct MADGINE_BULLET_EXPORT RigidBody : Scene::Entity::EntityComponent<RigidBody> {
        SERIALIZABLEUNIT(RigidBody)

        using Container = FreeListContainer<std::deque<Placeholder<0>>, Scene::Entity::EntityComponentFreeListConfig<RigidBody>>;

        RigidBody(Scene::Entity::Entity *entity);
        RigidBody(RigidBody &&other);
        ~RigidBody();

        RigidBody &operator=(RigidBody &&other);

        void init();
        void finalize();

        void update();

        btRigidBody *get();
        const btRigidBody *get() const;
        void activate();
        
        Scene::Entity::Transform *transform();

        float mass() const;
        void setMass(float mass);

        bool kinematic() const;
        void setKinematic(bool kinematic);

        bool ghost() const;
        void setGhost(bool ghost);

        float friction() const;
        void setFriction(float friction);

        Vector3 linearFactor() const;
        void setLinearFactor(const Vector3 &factor);

        Vector3 angularFactor() const;
        void setAngularFactor(const Vector3 &factor);

        Vector3 angularVelocity() const;
        void setAngularVelocity(const Vector3 &v);

        Vector3 velocity() const;
        void setVelocity(const Vector3 &v);

        void setOrientation(const Quaternion &q);

        uint16_t collisionGroup() const;
        void setCollisionGroup(uint16_t group);

        uint16_t collisionMask() const;
        void setCollisionMask(uint16_t group);

        void setShape(typename CollisionShapeManager::Handle handle);
        void setShapeName(std::string_view name);
        CollisionShapeManager::Resource *getShape() const;
        CollisionShapeInstance *getShapeInstance() const;

        PhysicsManager *mgr() const;

        friend struct PhysicsManager;

    protected:
        typename CollisionShapeManager::InstanceHandle mShapeHandle;
        struct Data;
        std::unique_ptr<Data> mData;
    };

    using RigidBodyPtr = Scene::Entity::EntityComponentPtr<RigidBody>;

}
}

REGISTER_TYPE(Engine::Physics::RigidBody)
REGISTER_TYPE(Engine::Scene::Entity::EntityComponentList<Engine::Physics::RigidBody>)