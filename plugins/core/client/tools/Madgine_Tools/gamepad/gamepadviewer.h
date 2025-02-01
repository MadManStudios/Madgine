#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "Madgine/render/textureloader.h"

namespace Engine {
namespace Tools {

    struct MADGINE_CLIENT_TOOLS_EXPORT GamepadViewer : Tool<GamepadViewer> {

        GamepadViewer(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual std::string_view key() const override;

        virtual void render() override;

    private:
        Render::TextureLoader::Handle mGamepadTexture;
    };

}
}

REGISTER_TYPE(Engine::Tools::GamepadViewer)