#include "../../../scenelib.h"

#include "mesh.h"

#include "Meta/math/boundingbox.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../entity.h"

NAMED_UNIQUECOMPONENT(Mesh, Engine::Scene::Entity::Mesh);

METATABLE_BEGIN_BASE(Engine::Scene::Entity::Mesh, Engine::Scene::Entity::EntityComponentBase)
    STORAGE_BEGIN(Engine::Scene::Entity::Mesh, Engine::Scene::Entity::Mesh)
    CONSTRUCTOR()
    STORAGE_END(Engine::Scene::Entity::Mesh)
    MEMBER(mMesh)
    MEMBER(mIsVisible)
    MEMBER(mInheritVisibility)
    MEMBER(mCastShadows)
    READONLY_PROPERTY(WorldVisible, worldVisible)
    MEMBER(mMaterial)
METATABLE_END(Engine::Scene::Entity::Mesh)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Mesh)
    FIELD(mMesh)
    FIELD(mMaterial)
    FIELD(mIsVisible)
    FIELD(mInheritVisibility)
    FIELD(mCastShadows)
SERIALIZETABLE_END(Engine::Scene::Entity::Mesh)

namespace Engine {
namespace Scene {
    namespace Entity {

        const Render::GPUMeshData *Mesh::data() const
        {
            return mMesh;
        }

        Math::AABB Mesh::aabb() const
        {
            return mMesh->mAABB;
        }

        bool Mesh::worldVisible(Contextual<Entity &> entity) const
        {
            if (!mIsVisible)
                return false;
            bool visible = true;
            if (mInheritVisibility) {
                EntityPtr parent = entity->parent();

                while (Execution::access_binding(parent, [&](Entity &e) {
                    Mesh *mesh = e.getComponent<Mesh>();
                    if (mesh) {
                        visible = mesh->worldVisible(e);
                        return false;
                    }
                    parent = e.parent();
                    return true;
                }))
                    ;
            }
            return visible;
        }

    }
}
}
