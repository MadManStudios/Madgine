#pragma once

#include "../entitycomponent.h"

#include "Madgine/meshloader/gpumeshloader.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Mesh : EntityComponent<Mesh> {

            using EntityComponent<Mesh>::EntityComponent;

            std::string_view getName() const;
            void setName(std::string_view name);

            const Render::GPUMeshData *data() const;
            uint32_t material() const;
            void setMaterial(uint32_t index);
            //void setMaterialName(std::string_view name);

            AABB aabb() const;

            void set(Render::GPUMeshLoader::Handle handle);            

            Render::GPUMeshLoader::Resource *get() const;
            const Render::GPUMeshLoader::Handle &handle() const;

            void setVisible(bool vis);
            bool isVisible() const;

            typename Render::GPUMeshLoader::Handle mMesh;
        private:            
            uint32_t mMaterial = 0;
            bool mIsVisible = true;
        };

        using MeshPtr = EntityComponentPtr<Mesh>;

    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::Mesh)
REGISTER_TYPE(Engine::Scene::Entity::EntityComponentList<Engine::Scene::Entity::Mesh>)