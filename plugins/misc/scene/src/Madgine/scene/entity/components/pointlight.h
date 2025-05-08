#pragma once

#include "../entitycomponent.h"

#include "Meta/math/color3.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT PointLight : EntityComponent<PointLight> {

            using EntityComponent<PointLight>::EntityComponent;

            uint32_t mRange = 10;
            Color3 mColor = { 1.0f, 1.0f, 1.0f };
        };

        using PointLightPtr = EntityComponentPtr<PointLight>;

    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::PointLight)
REGISTER_TYPE(Engine::Scene::Entity::EntityComponentList<Engine::Scene::Entity::PointLight>)