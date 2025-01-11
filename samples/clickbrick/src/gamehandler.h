#pragma once

#include "Madgine/input/widgethandler.h"

namespace ClickBrick {

    struct GameHandler : Engine::Input::WidgetHandler<GameHandler> {

        GameHandler(Engine::Input::UIManager &ui);        

        virtual std::string_view key() const override;		    

    };

}

REGISTER_TYPE(ClickBrick::GameHandler)