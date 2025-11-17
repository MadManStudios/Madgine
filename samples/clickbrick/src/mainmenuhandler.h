#pragma once

#include "Madgine/widgets/widgethandler.h"

namespace ClickBrick {

    struct MainMenuHandler : Engine::Widgets::WidgetHandler<MainMenuHandler> {
        SERIALIZABLEUNIT(MainMenuHandler)

        MainMenuHandler(Engine::Behavior::HandlerManager &ui);

        virtual std::string_view key() const override;

        virtual void setWidget(Engine::Widgets::WidgetBase *widget) override;

        void startGame();
    };

}