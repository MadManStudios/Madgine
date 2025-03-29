#pragma once

#include "Madgine/widgets/widgethandler.h"

namespace ClickBrick {

    struct GameHandler : Engine::Widgets::WidgetHandler<GameHandler> {

        GameHandler(Engine::HandlerManager &ui);        

        virtual std::string_view key() const override;		    

    };

}

REGISTER_TYPE(ClickBrick::GameHandler)