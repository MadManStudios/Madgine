#pragma once

#include "Madgine/resources/resourceloader.h"

namespace Engine {
namespace Window {

    struct LayoutDummy {
    };

    struct MADGINE_CLIENT_EXPORT LayoutLoader : Resources::ResourceLoader<LayoutLoader, LayoutDummy> {

        struct Interface : ResourceLoader::Interface {
            struct MADGINE_CLIENT_EXPORT Resource : ResourceLoader::Interface::Resource {
                using ResourceLoader::Interface::Resource::Resource;

                Serialize::FormattedSerializeStream readAsFormattedStream(Serialize::SerializeManager &mgr);
            };
        };

        LayoutLoader();

        Threading::Task<bool> loadImpl(LayoutDummy &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(LayoutDummy &data);

        virtual Threading::Task<Resources::BakeResult> bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir) override;
    };

}
}

REGISTER_TYPE(Engine::Window::LayoutLoader)