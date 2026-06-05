#pragma once

#include "Madgine/resources/instanceloader.h"

namespace Engine {
namespace Core {

    struct MADGINE_CLIENT_EXPORT LayoutLoader : Resources::InstanceLoader<LayoutLoader, MainWindow> {

        LayoutLoader();

        Threading::Task<bool> loadImpl(MainWindow &window, Resource *res);
        Threading::Task<void> unloadImpl(MainWindow &window);

        Threading::Task<Resources::BakeResult> bakeResources(std::vector<Platform::Filesystem::Path> &resourcesToBake, const Platform::Filesystem::Path &intermediateDir) override;
    };

}
}