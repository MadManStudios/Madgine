#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct ImGuiDemo : Tool<ImGuiDemo> {
        ImGuiDemo(ImRoot &root);

        void renderMenu() override;
        void render() override;

        std::string_view key() const override;
    };

}
}