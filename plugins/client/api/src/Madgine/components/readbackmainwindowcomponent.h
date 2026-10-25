#pragma once

#include "Madgine/render/camera.h"
#include "../window/mainwindowcomponent.h"
#include "../window/mainwindowcomponentcollector.h"

#include "Madgine/render/passes/readbackpass.h"

namespace Engine {
namespace Core {

    struct MADGINE_CLIENT_EXPORT ReadbackMainWindowComponent : MainWindowComponent<ReadbackMainWindowComponent> {

        SERIALIZABLEUNIT(ReadbackMainWindowComponent)

        ReadbackMainWindowComponent(Core::MainWindow &window);
        ~ReadbackMainWindowComponent();

        void render(Render::RenderTarget *target, size_t iteration) override;
        void setup(Render::RenderTarget *target) override;
        void shutdown(Render::RenderTarget *target) override;

        Render::ReadBackPass &pass();

    private:
        Render::ReadBackPass mPass;
    };

}
}