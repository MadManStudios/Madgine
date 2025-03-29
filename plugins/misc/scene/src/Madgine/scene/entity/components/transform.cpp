#include "../../../scenelib.h"

#include "transform.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/math/transformation.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"



ENTITYCOMPONENT_IMPL(Transform, Engine::Scene::Entity::Transform);

METATABLE_BEGIN(Engine::Scene::Entity::Transform)
MEMBER(mPosition)
MEMBER(mScale)
MEMBER(mOrientation)
METATABLE_END(Engine::Scene::Entity::Transform)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Transform)
FIELD(mParent)
FIELD(mPosition)
FIELD(mScale)
FIELD(mOrientation)
SERIALIZETABLE_END(Engine::Scene::Entity::Transform)

namespace Engine {

namespace Scene {
    namespace Entity {

        Matrix4 Transform::matrix() const
        {
            return TransformMatrix(mPosition, mScale, mOrientation);
        }

        Matrix4 Transform::worldMatrix() const
        {
            return parentMatrix() * matrix();
        }

        Matrix4 Transform::parentMatrix() const
        {
            if (mParent)
                return mParent->worldMatrix();
            else
                return Matrix4::IDENTITY;
        }

        void Transform::setParent(Transform *parent)
        {
            if (parent == this)
                return;
            Transform *ptr = parent;
            while (ptr) {
                Transform *next = ptr->mParent;
                if (next == this) {
                    ptr->setParent(nullptr);
                    ptr = nullptr;
                } else {
                    ptr = next;
                }
            }
            mParent = parent;
        }

        Transform *Transform::parent() const
        {
            return mParent;
        }
    }
}
}

