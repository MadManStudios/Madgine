#include "clickbricklib.h"

#include "mainmenuhandler.h"

#include "Meta/reflect/metatable_impl.h"

#include "Madgine/widgets/button.h"

#include "gamehandler.h"

#include "gamemanager.h"

UNIQUECOMPONENT(ClickBrick::MainMenuHandler)

METATABLE_BEGIN_BASE(ClickBrick::MainMenuHandler, Engine::Widgets::WidgetHandlerBase)
METATABLE_END(ClickBrick::MainMenuHandler)

namespace ClickBrick {

MainMenuHandler::MainMenuHandler(Engine::Behavior::HandlerManager &ui)
    : Engine::Widgets::WidgetHandler<MainMenuHandler>(ui, "MainMenu")
{
}

std::string_view MainMenuHandler::key() const
{
    return "MainMenuHandler";
}

void MainMenuHandler::setWidget(Engine::Widgets::WidgetBase *widget)
{
    WidgetHandlerBase::setWidget(widget);
    if (widget) {
        setupButton("StartGameButton", &MainMenuHandler::startGame, this);
    }
}

void MainMenuHandler::startGame()
{
    getHandler<GameManager>().start();
    getHandler<GameHandler>().open();
}

}