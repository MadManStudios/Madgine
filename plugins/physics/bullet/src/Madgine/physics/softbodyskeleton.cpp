#include "../bulletlib.h"

#include "softbodyskeleton.h"

#include "physicsmanager.h"

#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/scenemanager.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/scene/entity/entity.h"

#include "Meta/math/transformation.h"

#include "rigidbody.h"

#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

ENTITYCOMPONENT_IMPL(SoftBodySkeleton, Engine::Physics::SoftBodySkeleton)

METATABLE_BEGIN(Engine::Physics::SoftBodySkeleton)
/* PROPERTY(Mass, mass, setMass)
PROPERTY(Friction, friction, setFriction)
PROPERTY(Kinematic, kinematic, setKinematic)
PROPERTY(LinearFactor, linearFactor, setLinearFactor)
PROPERTY(AngularFactor, angularFactor, setAngularFactor)
PROPERTY(Shape, getShape, setShape)
READONLY_PROPERTY(ShapeData, getShapeInstance)*/
METATABLE_END(Engine::Physics::SoftBodySkeleton)

SERIALIZETABLE_BEGIN(Engine::Physics::SoftBodySkeleton)
/* FIELD(mShapeHandle)
ENCAPSULATED_FIELD(Kinematic, kinematic, setKinematic)
ENCAPSULATED_FIELD(Mass, mass, setMass)
ENCAPSULATED_FIELD(Friction, friction, setFriction)
ENCAPSULATED_FIELD(LinearFactor, linearFactor, setLinearFactor)
ENCAPSULATED_FIELD(AngularFactor, angularFactor, setAngularFactor)*/
SERIALIZETABLE_END(Engine::Physics::SoftBodySkeleton)

namespace Engine {
namespace Physics {

    struct SoftBodySkeleton::Data {

        Data() = default;

        void setup(SoftBodySkeleton *component, PhysicsManager *mgr, Scene::Entity::Transform *transform)
        {
            mMgr = mgr;
            mTransform = transform;
            mSoftBody = std::unique_ptr<btSoftBody> { btSoftBodyHelpers::CreateRope(mgr->worldInfo(), { 0, 0, 0 }, { 0, 0.5f, 0 }, 5, 0) };
        }

        void add()
        {
            if (!mSoftBody->getUserIndex()) {
                for (size_t i = 0; i < mSoftBody->m_anchors.size(); ++i) {
                    btRigidBody *rigidbody = mSoftBody->m_anchors[i].m_body;
                    if (!rigidbody->getUserIndex())
                        return;
                }
                mMgr->world().addSoftBody(mSoftBody.get());
                mSoftBody->setUserIndex(1);
            }
        }

        void remove()
        {
            if (mSoftBody->getUserIndex()) {
                mMgr->world().removeSoftBody(mSoftBody.get());
                mSoftBody->setUserIndex(0);
            }
        }

        Scene::Entity::Transform *mTransform = nullptr;
        PhysicsManager *mMgr = nullptr;

        std::unique_ptr<btSoftBody> mSoftBody;
    };

    SoftBodySkeleton::SoftBodySkeleton(Scene::Entity::Entity *entity)
        : Scene::Entity::EntityComponent<SoftBodySkeleton>(entity)        
    {
        mData = std::make_unique<Data>();
    }

    SoftBodySkeleton::SoftBodySkeleton(SoftBodySkeleton &&other) = default;

    SoftBodySkeleton::~SoftBodySkeleton() = default;

    SoftBodySkeleton &SoftBodySkeleton::operator=(SoftBodySkeleton &&) = default;

    void SoftBodySkeleton::init()
    {
        mData->setup(this, &entity()->sceneMgr().getComponent<PhysicsManager>(), entity()->addComponent<Scene::Entity::Transform>());

        mData->add();
    }

    void SoftBodySkeleton::finalize()
    {
        mData->remove();
    }

    void SoftBodySkeleton::update()
    {
        if (!get()->getUserIndex()) {
            mData->add();
        }
    }

    btSoftBody *SoftBodySkeleton::get()
    {
        return mData->mSoftBody.get();
    }

    void SoftBodySkeleton::activate()
    {
        get()->activate(true);
    }

    void SoftBodySkeleton::attach(RigidBody *rigidbody, size_t index, const Engine::Vector3 &offset)
    {
        bool wasAdded = get()->getUserIndex();
        if (wasAdded && !rigidbody->get()->getUserIndex())
            mData->remove();
        get()->appendAnchor(index, rigidbody->get(), { offset.x, offset.y, offset.z });
    }

}
}
