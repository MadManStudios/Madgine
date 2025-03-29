#pragma once

#include "Madgine/widgets/widgethandler.h"

namespace ClickBrick {

    struct MainMenuHandler : Engine::Widgets::WidgetHandler<MainMenuHandler> {
        SERIALIZABLEUNIT(MainMenuHandler)

        MainMenuHandler(Engine::HandlerManager &ui);

        virtual std::string_view key() const override;

        virtual void startLifetime() override;

        void startGame();
    };

}

REGISTER_TYPE(ClickBrick::MainMenuHandler)