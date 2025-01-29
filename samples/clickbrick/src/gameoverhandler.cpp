#include "clickbricklib.h"

#include "gameoverhandler.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/widgets/widget.h"

#include "Madgine/widgets/label.h"

#include "gamemanager.h"

UNIQUECOMPONENT(ClickBrick::GameOverHandler)

METATABLE_BEGIN_BASE(ClickBrick::GameOverHandler, Engine::Widgets::WidgetHandlerBase)
METATABLE_END(ClickBrick::GameOverHandler)

namespace ClickBrick {

    GameOverHandler::GameOverHandler(Engine::HandlerManager &ui)
    : Engine::Widgets::WidgetHandler<GameOverHandler>(ui, "GameOver", Engine::Widgets::WidgetHandlerBase::WidgetType::MODAL_OVERLAY)
    {
    }

    std::string_view GameOverHandler::key() const
    {
        return "GameOverHandler";
    }

    void GameOverHandler::startLifetime()
    {
        WidgetHandlerBase::startLifetime();
        if (widget()) {
            mLifetime.attach(widget()->pointerClickEvent().connect(&GameOverHandler::restartGame, this));
        }
    }

    void ClickBrick::GameOverHandler::setWidget(Engine::Widgets::WidgetBase *widget)
    {
        WidgetHandlerBase::setWidget(widget);
        if (widget) {         
            mScoreLabel = widget->getChildRecursive<Engine::Widgets::Label>("ScoreLabel");
        } else {
            mScoreLabel = nullptr;
        }
    }

    void GameOverHandler::restartGame()
    {
        getHandler<GameManager>().start();
        close();
    }

    void GameOverHandler::setScore(int score)
    {
        mScoreLabel->mText = "Your Score: " + std::to_string(score);
    }

}