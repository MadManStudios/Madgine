#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct ImGuiTestEngine : Tool<ImGuiTestEngine> {
        ImGuiTestEngine(ImRoot &root);

        void renderMenu() override;
        void render() override;

        std::string_view key() const override;
    };

}
}