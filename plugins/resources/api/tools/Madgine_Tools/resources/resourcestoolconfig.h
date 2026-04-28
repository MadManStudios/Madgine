#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct ResourcesToolConfig : public Tool<ResourcesToolConfig> {

        SERIALIZABLEUNIT(ResourcesToolConfig)

        ResourcesToolConfig(ImRoot &root);

        virtual Threading::Task<bool> init() override;

        virtual void renderMenu() override;
        virtual bool renderConfiguration(const Filesystem::Path &config) override;
        virtual void loadConfiguration(const Filesystem::Path &config) override;
        virtual void saveConfiguration(const Filesystem::Path &config) override;

        std::string_view key() const override;

    private:
        std::map<std::pair<std::string, Filesystem::Path>, std::vector<Resources::ResourceBase *>> mResourceCache;
        std::set<std::pair<std::string, Filesystem::Path>> mResourceConfig;
    };

}
}