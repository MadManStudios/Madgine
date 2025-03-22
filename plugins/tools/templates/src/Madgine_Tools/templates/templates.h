#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TEMPLATES_EXPORT Templates : Tool<Templates> {

        Templates(ImRoot &root);

        Threading::Task<bool> init() override;

        void renderMenu() override;

        std::string_view key() const override;

    private:
        Inspector *mInspector;
    };

}
}

REGISTER_TYPE(Engine::Tools::Templates)