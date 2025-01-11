#include "clickbricklib.h"

#include "mainmenuhandler.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/widgets/button.h"

#include "gamehandler.h"

#include "gamemanager.h"

UNIQUECOMPONENT(ClickBrick::MainMenuHandler)

METATABLE_BEGIN_BASE(ClickBrick::MainMenuHandler, Engine::Input::WidgetHandlerBase)
METATABLE_END(ClickBrick::MainMenuHandler)

namespace ClickBrick {

MainMenuHandler::MainMenuHandler(Engine::Input::UIManager &ui)
    : Engine::Input::WidgetHandler<MainMenuHandler>(ui, "MainMenu", Engine::Input::WidgetHandlerBase::WidgetType::ROOT_WIDGET)
{
}

std::string_view MainMenuHandler::key() const
{
    return "MainMenuHandler";
}

void MainMenuHandler::startLifetime()
{
    WidgetHandlerBase::startLifetime();
    if (widget()) {
        setupButton("StartGameButton", &MainMenuHandler::startGame, this);
    }
}

void MainMenuHandler::startGame()
{
    getHandler<GameManager>().start();
    getHandler<GameHandler>().open();
}

}