#include "../bulletlib.h"

#include "physicsmanager.h"

#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/scenemanager.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/scene/entity/entity.h"

#include "rigidbody.h"

#include "rigidbodydata.h"

ENTITYCOMPONENT_IMPL(RigidBody, Engine::Physics::RigidBody)

METATABLE_BEGIN(Engine::Physics::RigidBody)
PROPERTY(Mass, mass, setMass)
PROPERTY(Friction, friction, setFriction)
PROPERTY(Kinematic, kinematic, setKinematic)
PROPERTY(LinearFactor, linearFactor, setLinearFactor)
PROPERTY(AngularFactor, angularFactor, setAngularFactor)
PROPERTY(Shape, getShape, setShape)
READONLY_PROPERTY(ShapeData, getShapeInstance)
PROPERTY(Ghost, ghost, setGhost)
PROPERTY(CollisionGroup, collisionGroup, setCollisionGroup)
PROPERTY(CollisionMask, collisionMask, setCollisionMask)
METATABLE_END(Engine::Physics::RigidBody)

SERIALIZETABLE_BEGIN(Engine::Physics::RigidBody)
FIELD(mShapeHandle)
ENCAPSULATED_FIELD(Kinematic, kinematic, setKinematic)
ENCAPSULATED_FIELD(Mass, mass, setMass)
ENCAPSULATED_FIELD(Friction, friction, setFriction)
ENCAPSULATED_FIELD(LinearFactor, linearFactor, setLinearFactor)
ENCAPSULATED_FIELD(AngularFactor, angularFactor, setAngularFactor)
ENCAPSULATED_FIELD(Ghost, ghost, setGhost)
ENCAPSULATED_FIELD(CollisionGroup, collisionGroup, setCollisionGroup)
ENCAPSULATED_FIELD(CollisionMask, collisionMask, setCollisionMask)
SERIALIZETABLE_END(Engine::Physics::RigidBody)

namespace Engine {
namespace Physics {

    RigidBody::RigidBody(Scene::Entity::Entity *entity)
        : Scene::Entity::EntityComponent<RigidBody>(entity)
    {
        mData = std::make_unique<Data>(this);
    }

    RigidBody::RigidBody(RigidBody &&other)
        : NamedComponent(std::move(other))
        , mShapeHandle(std::move(other.mShapeHandle))
        , mData(std::move(other.mData))
    {
        if (mData)
            get()->setUserPointer(this);
    }

    RigidBody::~RigidBody() = default;

    RigidBody &RigidBody::operator=(RigidBody &&other)
    {
        NamedComponent::operator=(std::move(other));
        std::swap(mShapeHandle, other.mShapeHandle);
        std::swap(mData, other.mData);
        if (mData)
            get()->setUserPointer(this);
        return *this;
    }

    void RigidBody::init()
    {
        Scene::Entity::Transform *transform = entity()->addComponent<Scene::Entity::Transform>();

        mData->mTransform = transform;

        mData->mMgr = &entity()->sceneMgr().getComponent<PhysicsManager>();

        get()->saveKinematicState(1.0f);
        get()->saveKinematicState(1.0f);

        if (mShapeHandle && mShapeHandle->available()) {
            get()->setCollisionShape(mShapeHandle->get());
        }

        mData->add();
    }

    void RigidBody::finalize()
    {
        mData->remove();
    }

    void RigidBody::update()
    {
        if (mShapeHandle && mShapeHandle->available() && !get()->getCollisionShape()) {
            get()->setCollisionShape(mShapeHandle->get());
            float mass = get()->getMass();
            btVector3 inertia;
            mShapeHandle->get()->calculateLocalInertia(mass, inertia);
            get()->setMassProps(mass, inertia);
            mData->add();
        }
    }

    btRigidBody *RigidBody::get()
    {
        return &mData->mRigidBody;
    }

    const btRigidBody *RigidBody::get() const
    {
        return &mData->mRigidBody;
    }

    void RigidBody::activate()
    {
        get()->activate(true);
    }

    Scene::Entity::Transform *RigidBody::transform()
    {
        return mData->mTransform;
    }

    float RigidBody::mass() const
    {
        return get()->getMass();
    }

    void RigidBody::setMass(float mass)
    {
        float oldMass = get()->getMass();
        if (mass != oldMass) {
            if (get()->getCollisionShape()) {
                mData->remove();
                btVector3 inertia;
                get()->getCollisionShape()->calculateLocalInertia(mass, inertia);
                get()->setMassProps(mass, inertia);
                mData->add();
            } else {
                get()->setMassProps(mass, {});
            }
        }
    }

    bool RigidBody::kinematic() const
    {
        return get()->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT;
    }

    void RigidBody::setKinematic(bool kinematic)
    {
        if (kinematic) {
            get()->setCollisionFlags(get()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            get()->setActivationState(DISABLE_DEACTIVATION);
            get()->setCollisionFlags(get()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        } else {
            get()->setCollisionFlags(get()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            get()->forceActivationState(ISLAND_SLEEPING);
            if (get()->getMass() > 0) {
                mData->remove();
                mData->add();
            }
        }
    }

    bool RigidBody::ghost() const
    {
        return get()->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE;
    }

    void RigidBody::setGhost(bool ghost)
    {
        if (ghost) {
            get()->setCollisionFlags(get()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        } else {
            get()->setCollisionFlags(get()->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
    }

    float RigidBody::friction() const
    {
        return get()->getFriction();
    }

    void RigidBody::setFriction(float friction)
    {
        get()->setFriction(friction);
    }

    Vector3 RigidBody::linearFactor() const
    {
        return get()->getLinearFactor().m_floats;
    }

    void RigidBody::setLinearFactor(const Vector3 &factor)
    {
        get()->setLinearFactor({ factor.x, factor.y, factor.z });
    }

    Vector3 RigidBody::angularFactor() const
    {
        return get()->getAngularFactor().m_floats;
    }

    void RigidBody::setAngularFactor(const Vector3 &factor)
    {
        get()->setAngularFactor({ factor.x, factor.y, factor.z });
    }

    Vector3 RigidBody::angularVelocity() const
    {
        return get()->getAngularVelocity().m_floats;
    }

    void RigidBody::setAngularVelocity(const Vector3 &v)
    {
        get()->setAngularVelocity({ v.x, v.y, v.z });
    }

    Vector3 RigidBody::velocity() const
    {
        return get()->getLinearVelocity().m_floats;
    }

    void RigidBody::setVelocity(const Vector3 &v)
    {
        get()->setLinearVelocity({ v.x, v.y, v.z });
    }

    void RigidBody::setOrientation(const Quaternion &q)
    {
        btTransform t = get()->getWorldTransform();
        t.setRotation({ q.x, q.y, q.z, q.w });
        get()->setWorldTransform(t);
    }

    uint16_t RigidBody::collisionGroup() const
    {
        return mData->collisionGroup();
    }

    void RigidBody::setCollisionGroup(uint16_t group)
    {
        mData->setCollisionGroup(group);
    }

    uint16_t RigidBody::collisionMask() const
    {
        return mData->collisionMask();
    }

    void RigidBody::setCollisionMask(uint16_t mask)
    {
        mData->setCollisionMask(mask);
    }

    void RigidBody::setShape(typename CollisionShapeManager::Handle handle)
    {
        mData->remove();

        mShapeHandle = std::move(handle);
        if (mShapeHandle->available()) {
            get()->setCollisionShape(mShapeHandle->get());
            btVector3 inertia;
            get()->getCollisionShape()->calculateLocalInertia(mass(), inertia);
            get()->setMassProps(mass(), inertia);
            mData->add();
        } else {
            get()->setCollisionShape(nullptr);
        }
    }

    void RigidBody::setShapeName(std::string_view name)
    {
        mData->remove();

        mShapeHandle.load(name);
        if (mShapeHandle->available()) {
            get()->setCollisionShape(mShapeHandle->get());
            btVector3 inertia;
            get()->getCollisionShape()->calculateLocalInertia(mass(), inertia);
            get()->setMassProps(mass(), inertia);
            mData->add();
        } else {
            get()->setCollisionShape(nullptr);
        }
    }

    CollisionShapeManager::Resource *RigidBody::getShape() const
    {
        return mShapeHandle ? mShapeHandle.resource() : nullptr;
    }

    CollisionShapeInstance *RigidBody::getShapeInstance() const
    {
        return mShapeHandle;
    }

    PhysicsManager *RigidBody::mgr() const
    {
        return mData->mMgr;
    }

}
}
