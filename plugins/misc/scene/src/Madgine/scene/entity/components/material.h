#pragma once

#include "Madgine/meshloader/gpumeshdata.h"

#include "../entitycomponent.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Material : EntityComponent<Material> {

            using EntityComponent<Material>::EntityComponent;

            const Render::GPUMeshData::Material *get() const;

            Render::GPUMeshData::Material mMaterial;
        };

    }
}
}