#include "clickbricklib.h"

#include "mainmenuhandler.h"

#include "Madgine/widgets/button.h"
#include "Madgine/widgets/label.h"

#include "Meta/reflect/metatable_impl.h"

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
        setupButton("StartGameButton", &MainMenuHandler::startGame, this)->mText = "";

        for (size_t i = 6; i < 32; ++i) {
            Engine::Widgets::Label *label = widget->createChild<Engine::Widgets::Label>();
            label->mText = std::to_string(i) + ":  Test String, with g W M y";
            label->setPos({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, static_cast<float>(i + 1) * i * 0.7f - 12.0f, 0.0f, 0.0f, 1.0f });
            label->setSize({ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, static_cast<float>(2 * i), 0.0f, 0.0f, 1.0f });
            label->mTextRenderData.mPivot = { 0.0f, 0.0f };
            label->mTextRenderData.mFontSize = i;
            label->mTextRenderData.setFontName("OpenSans");

            label = widget->createChild<Engine::Widgets::Label>();
            label->mText = std::to_string(i) + ":  Test String, with g W M y";
            label->setPos({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, static_cast<float>(i + 1) * i * 0.7f - 12.0f, 0.0f, 0.0f, 1.0f });
            label->setSize({ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, static_cast<float>(2 * i), 0.0f, 0.0f, 1.0f });
            label->mTextRenderData.mPivot = { 1.0f, 0.0f };
            label->mTextRenderData.mFontSize = i;
            label->mTextRenderData.setFontName("OpenSans");
        }
    }
}

void MainMenuHandler::startGame()
{
    getHandler<GameManager>().start();
    getHandler<GameHandler>().open();
}

}