#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#if ENABLE_PLUGINS

namespace Engine {
namespace Tools {

    struct PluginExporter : Core::RootComponent<PluginExporter> {

        PluginExporter(Core::Root &root);

        static void exportStaticComponentHeader(const Platform::Filesystem::Path &outFile);

        virtual std::string_view key() const override;

        virtual Threading::Task<int> runTools() override;
    };

}
}

#endif