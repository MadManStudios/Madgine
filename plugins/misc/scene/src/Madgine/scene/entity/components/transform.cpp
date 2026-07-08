#include "../../../scenelib.h"

#include "transform.h"

#include "Meta/math/transformation.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../entity.h"

NAMED_UNIQUECOMPONENT(Transform, Engine::Scene::Entity::Transform);

METATABLE_BEGIN(Engine::Scene::Entity::Transform)
    CONSTRUCTOR()
    MEMBER(mPosition)
    MEMBER(mScale)
    MEMBER(mOrientation)
METATABLE_END(Engine::Scene::Entity::Transform)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Transform)
    FIELD(mPosition)
    FIELD(mScale)
    FIELD(mOrientation)
SERIALIZETABLE_END(Engine::Scene::Entity::Transform)

namespace Engine {

namespace Scene {
    namespace Entity {

        Math::Matrix4 Transform::matrix() const
        {
            return TransformMatrix(mPosition, mScale, mOrientation);
        }

        Math::Matrix4 Transform::worldMatrix(Entity &entity) const
        {
            return parentMatrix(entity) * matrix();
        }

        Math::Matrix4 Transform::parentMatrix(Entity &entity) const
        {
            Math::Matrix4 result = Math::Matrix4::IDENTITY;
            Execution::access_binding(entity.parent(), [&](Entity &e) {
                result = e.getComponent<Transform>()->worldMatrix(e); });
            return result;
        }

        Math::Vector3 Transform::worldPosition(Entity &entity) const
        {
            return worldMatrix(entity).GetColumn(3).xyz();
        }

        Math::Quaternion Transform::worldOrientation(Entity &entity) const
        {
            Math::Quaternion result;
            Execution::access_binding(entity.parent(), [&](Entity &e) { result = e.getComponent<Transform>()->worldOrientation(e); });
            return result * mOrientation;
        }


    }
}
}
