#pragma once

#include "Madgine/resources/resourceloader.h"

#include "animationdescriptor.h"

namespace Engine {
namespace Render {

    struct MADGINE_ANIMATIONLOADER_EXPORT AnimationLoader : Resources::ResourceLoader<AnimationLoader, AnimationList, std::list<Placeholder<0>>> {

        using Base = ResourceLoader<AnimationLoader, AnimationList, std::list<Placeholder<0>>>;

        AnimationLoader();

        Threading::Task<bool> loadImpl(AnimationList &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(AnimationList &data);

    };

}
}

REGISTER_TYPE(Engine::Render::AnimationLoader)