#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_ROOT_TOOLS_EXPORT KeyValueRegistry : Tool<KeyValueRegistry> {

        KeyValueRegistry(ImRoot &root);

        Threading::Task<bool> init() override;

        void render() override;

        std::string_view key() const override;

    private:
        Inspector *mInspector;
    };

}
}

REGISTER_TYPE(Engine::Tools::KeyValueRegistry)