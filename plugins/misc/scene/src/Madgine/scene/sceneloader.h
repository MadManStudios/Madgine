#pragma once

#include "Madgine/resources/instanceloader.h"

namespace Engine {
namespace Scene {

    struct MADGINE_SCENE_EXPORT SceneLoader : Resources::InstanceLoader<SceneLoader, SceneContainer> {
        SceneLoader();

        Threading::Task<bool> loadImpl(SceneContainer &container, Resource *res);
    };

}
}