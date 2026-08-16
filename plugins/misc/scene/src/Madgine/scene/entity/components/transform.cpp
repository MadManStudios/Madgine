#include "../../../scenelib.h"

#include "transform.h"

#include "Meta/math/transformation.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/type/storageops_impl.h"

#include "../entity.h"

NAMED_UNIQUECOMPONENT(Transform, Engine::Scene::Entity::Transform);

METATABLE_BEGIN(Engine::Scene::Entity::Transform)
    MEMBER(mPosition)
    MEMBER(mScale)
    MEMBER(mOrientation)
    PROPERTY(WorldPosition, worldPosition, setWorldPosition)
METATABLE_END(Engine::Scene::Entity::Transform)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Transform)
    FIELD(mPosition)
    FIELD(mScale)
    FIELD(mOrientation)
SERIALIZETABLE_END(Engine::Scene::Entity::Transform)

STORAGEOPS_BEGIN(Engine::Scene::Entity::Transform)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Scene::Entity::Transform)

namespace Engine {

namespace Scene {
    namespace Entity {

        Math::Matrix4 Transform::matrix() const
        {
            return TransformMatrix(mPosition, mScale, mOrientation);
        }

        Math::Matrix4 Transform::worldMatrix(Contextual<Entity &> entity) const
        {
            return parentMatrix(entity) * matrix();
        }

        Math::Matrix4 Transform::parentMatrix(Contextual<Entity &> entity) const
        {
            Math::Matrix4 result = Math::Matrix4::IDENTITY;
            Execution::access_binding(entity->parent(), [&](Entity &e) { result = e.getComponent<Transform>()->worldMatrix(e); });
            return result;
        }

        Math::Vector3 Transform::worldPosition(Contextual<Entity &> entity) const
        {
            return worldMatrix(entity).GetColumn(3).xyz();
        }

        void Transform::setWorldPosition(const Math::Vector3 &position, Contextual<Entity &> entity)
        {
            Math::Matrix4 parentMat = parentMatrix(entity);
            Math::Matrix4 invParentMat = parentMat.Inverse();
            Math::Vector3 localPos = (invParentMat * Math::Vector4(position, 1.0f)).xyz();
            mPosition = localPos;
        }

        Math::Quaternion Transform::worldOrientation(Contextual<Entity &> entity) const
        {
            Math::Quaternion result;
            Execution::access_binding(entity->parent(), [&](Entity &e) { result = e.getComponent<Transform>()->worldOrientation(e); });
            return result * mOrientation;
        }

        void Transform::setWorldOrientation(const Math::Quaternion orientation, Contextual<Entity &> entity)
        {
            Math::Quaternion parentOrientation;
            Execution::access_binding(entity->parent(), [&](Entity &e) { parentOrientation = e.getComponent<Transform>()->worldOrientation(e); });
            Math::Quaternion invParentOrientation = parentOrientation.inverse();
            Math::Quaternion localOrientation = invParentOrientation * orientation;
            mOrientation = localOrientation;
        }

    }
}
}
