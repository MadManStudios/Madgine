#pragma once

#include "../toolscollector.h"
#include "../toolbase.h"

namespace Engine {
namespace Tools {

    struct ImGuiDemo : Tool<ImGuiDemo> {
        ImGuiDemo(ImRoot &root);

        void render() override;

        std::string_view key() const override;
    };

}
}

REGISTER_TYPE(Engine::Tools::ImGuiDemo)