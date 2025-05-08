#pragma once

#include "../entitycomponent.h"

#include "Madgine/meshloader/gpumeshdata.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Material : EntityComponent<Material> {

            using EntityComponent<Material>::EntityComponent;

            const Render::GPUMeshData::Material *get() const;

            Render::GPUMeshData::Material mMaterial;
        };

        using MaterialPtr = EntityComponentPtr<Material>;

    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::Material);
REGISTER_TYPE(Engine::Scene::Entity::EntityComponentList<Engine::Scene::Entity::Material>);