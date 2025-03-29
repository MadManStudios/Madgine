#pragma once

#include "Madgine/resources/resourceloader.h"

#include "meshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_MESHLOADER_EXPORT MeshLoader : Resources::ResourceLoader<MeshLoader, MeshData> {

        using Base = ResourceLoader<MeshLoader, MeshData>;

        MeshLoader();

        Threading::Task<bool> loadImpl(MeshData &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(MeshData &data);

        
        
    };

}
}

REGISTER_TYPE(Engine::Render::MeshLoader)