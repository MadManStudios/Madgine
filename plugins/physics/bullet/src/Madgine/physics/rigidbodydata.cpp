#include "../bulletlib.h"

#include "rigidbodydata.h"

#include "Madgine/scene/entity/components/transform.h"

#include "Meta/math/transformation.h"

#include "physicsmanager.h"

namespace Engine {
namespace Physics {

    RigidBody::Data::Data(RigidBody *component, Scene::Entity::Transform *transform)
        : mRigidBody(btRigidBody::btRigidBodyConstructionInfo { 0.0f, this, nullptr, { 0.0f, 0.0f, 0.0f } })
        , mTransform(transform)
    {
        mRigidBody.setUserPointer(component);
        mRigidBody.setUserIndex(0);
    }

    void RigidBody::Data::setWorldTransform(const btTransform &transform)
    {
        if (mTransform) {
            Matrix4 p = mTransform->parentMatrix();

            btQuaternion q = transform.getRotation();

            Matrix4 m = p.Inverse() * TransformMatrix(Vector3 { transform.getOrigin() }, Vector3::UNIT_SCALE, Quaternion { q.x(), q.y(), q.z(), q.w() });

            std::tie(mTransform->mPosition, mTransform->mScale, mTransform->mOrientation) = DecomposeTransformMatrix(m);
        }
    }

    void RigidBody::Data::getWorldTransform(btTransform &transform) const
    {
        if (mTransform) {
            Matrix4 m = mTransform->worldMatrix();

            Vector3 pos;
            Vector3 scale;
            Quaternion orientation;
            std::tie(pos, scale, orientation) = DecomposeTransformMatrix(m);
            transform = btTransform { { orientation.x, orientation.y, orientation.z, orientation.w }, { pos.x, pos.y, pos.z } };
        } else {
            transform = btTransform { { 0.0f, 0.0f, 0.0f, 1.0f } };
        }
    }

    uint16_t RigidBody::Data::collisionGroup() const
    {
        return mCollisionGroup;
    }

    void RigidBody::Data::setCollisionGroup(uint16_t group)
    {
        if (mRigidBody.getBroadphaseProxy())
            mRigidBody.getBroadphaseProxy()->m_collisionFilterGroup = group;
        mCollisionGroup = group;
    }

    uint16_t RigidBody::Data::collisionMask() const
    {
        return mCollisionMask;
    }

    void RigidBody::Data::setCollisionMask(uint16_t mask)
    {
        if (mRigidBody.getBroadphaseProxy())
            mRigidBody.getBroadphaseProxy()->m_collisionFilterMask = mask;
        mCollisionMask = mask;
    }

    void RigidBody::Data::add()
    {
        if (mMgr && mRigidBody.getCollisionShape() && !mRigidBody.getUserIndex()) {
            mMgr->world().addRigidBody(&mRigidBody, mCollisionGroup, mCollisionMask);
            mRigidBody.activate(true);
            mRigidBody.setUserIndex(1);
            mRigidBody.clearForces();
        }
    }

    void RigidBody::Data::remove()
    {
        if (mRigidBody.getUserIndex()) {
            mMgr->world().removeRigidBody(&mRigidBody);
            mRigidBody.setUserIndex(0);
        }
    }

}
}