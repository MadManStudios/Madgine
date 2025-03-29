#include "clickbricklib.h"

#include "gamehandler.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

UNIQUECOMPONENT(ClickBrick::GameHandler)

METATABLE_BEGIN_BASE(ClickBrick::GameHandler, Engine::Widgets::WidgetHandlerBase)
METATABLE_END(ClickBrick::GameHandler)

namespace ClickBrick {

GameHandler::GameHandler(Engine::HandlerManager &ui)
    : Engine::Widgets::WidgetHandler<GameHandler>(ui, "Ingame", Engine::Widgets::WidgetHandlerBase::WidgetType::ROOT_WIDGET)
{
}

std::string_view GameHandler::key() const
{
    return "GameHandler";
}

}
