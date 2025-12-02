#pragma once

#include "Madgine/resources/instanceloader.h"

namespace Engine {
namespace Window {

    struct MADGINE_CLIENT_EXPORT LayoutLoader : Resources::InstanceLoader<LayoutLoader, MainWindow> {

        LayoutLoader();

        Threading::Task<bool> loadImpl(MainWindow &window, Resource *res);
        Threading::Task<void> unloadImpl(MainWindow &window);

        virtual Threading::Task<Resources::BakeResult> bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir) override;
    };

}
}