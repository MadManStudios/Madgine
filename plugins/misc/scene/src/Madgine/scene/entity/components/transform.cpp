#include "../../../scenelib.h"

#include "transform.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/math/transformation.h"

#include "../entity.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

NAMED_UNIQUECOMPONENT(Transform, Engine::Scene::Entity::Transform);

METATABLE_BEGIN(Engine::Scene::Entity::Transform)
MEMBER(mPosition)
MEMBER(mScale)
MEMBER(mOrientation)
METATABLE_END(Engine::Scene::Entity::Transform)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Transform)
//FIELD(mParent) //TODO
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
            Matrix4 result = Matrix4::IDENTITY;
            mParent.access([&](Entity &e) { result = e.getComponent<Transform>()->worldMatrix(); });
            return result;
        }

        void Transform::setParent(EntityPtr parent)
        {
            if (parent == entity().pointer())
                return;
            EntityPtr ptr = parent;
            while (ptr.access([&](Entity &e) {
                        EntityPtr next = e.getComponent<Transform>()->mParent;
                        ptr = next;
                        if (next == entity().pointer()) {
                            e.getComponent<Transform>()->setParent({});
                            return false;
                        } else {                            
                            return true;
                        } }))
                ;
            mParent = parent;
        }

        const EntityPtr &Transform::parent() const
        {
            return mParent;
        }
    }
}
}
