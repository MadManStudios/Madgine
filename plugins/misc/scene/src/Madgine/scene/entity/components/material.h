#pragma once

#include "Madgine/meshloader/gpumeshdata.h"

#include "../entitycomponent.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Material : EntityComponent<Material> {

            Material() = default;

            Material(const Material&) {
                throw 0;
            }

            Material &operator=(const Material&) {
                throw 0;
            }

            Render::GPUMeshData::Material mMaterial;
        };

    }
}
}