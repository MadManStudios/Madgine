#include "../../../scenelib.h"

#include "mesh.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/math/boundingbox.h"

#include "Meta/keyvalue/valuetype.h"

ENTITYCOMPONENT_IMPL(Mesh, Engine::Scene::Entity::Mesh);

METATABLE_BEGIN(Engine::Scene::Entity::Mesh)
MEMBER(mMesh)
PROPERTY(Visible, isVisible, setVisible)
PROPERTY(Material, material, setMaterial)
METATABLE_END(Engine::Scene::Entity::Mesh)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Mesh)
FIELD(mMesh)
ENCAPSULATED_FIELD(Material, material, setMaterial)
SERIALIZETABLE_END(Engine::Scene::Entity::Mesh)

namespace Engine {
namespace Scene {
    namespace Entity {

        const Render::GPUMeshData *Mesh::data() const
        {
            return mMesh;
        }

        uint32_t Mesh::material() const
        {
            return mMaterial;
        }

        void Mesh::setMaterial(uint32_t material)
        {
            mMaterial = material;
        }

        /* void Mesh::setMaterialName(std::string_view name)
        {
            if (!mMesh)
                return;
            auto it = std::find_if(mMesh->mMaterials.begin(), mMesh->mMaterials.end(), [&](const Render::GPUMeshData::Material &mat) { return mat.mName == name; });
            if (it != mMesh->mMaterials.end())
                mMaterial = &*it;
        }*/

        std::string_view Mesh::getName() const
        {
            return mMesh.name();
        }

        void Mesh::setName(std::string_view name)
        {
            mMesh.load(name);
            mMaterial = 0;
        }

        void Mesh::setVisible(bool vis)
        {
            mIsVisible = vis;
        }

        bool Mesh::isVisible() const
        {
            return mIsVisible;
        }

        AABB Mesh::aabb() const
        {
            return mMesh->mAABB;
        }

        void Mesh::set(Render::GPUMeshLoader::Handle handle)
        {
            mMesh = std::move(handle);
            mMaterial = 0;
        }

        Render::GPUMeshLoader::Resource *Mesh::get() const
        {
            return mMesh.resource();
        }

        const Render::GPUMeshLoader::Handle &Engine::Scene::Entity::Mesh::handle() const
        {
            return mMesh;
        }
    }
}
}
