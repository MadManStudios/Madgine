#include "../../../scenelib.h"

#include "mesh.h"

#include "Meta/math/boundingbox.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

NAMED_UNIQUECOMPONENT(Mesh, Engine::Scene::Entity::Mesh);

METATABLE_BEGIN(Engine::Scene::Entity::Mesh)
    MEMBER(mMesh)
    MEMBER(mIsVisible)
    MEMBER(mMaterial)
METATABLE_END(Engine::Scene::Entity::Mesh)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Mesh)
    FIELD(mMesh)
    FIELD(mMaterial)
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

    }
}
}
