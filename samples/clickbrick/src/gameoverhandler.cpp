#include "clickbricklib.h"

#include "gameoverhandler.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/widgets/widget.h"

#include "Madgine/widgets/label.h"

#include "gamemanager.h"

UNIQUECOMPONENT(ClickBrick::GameOverHandler)

METATABLE_BEGIN_BASE(ClickBrick::GameOverHandler, Engine::Input::HandlerBase)
METATABLE_END(ClickBrick::GameOverHandler)

namespace ClickBrick {

    GameOverHandler::GameOverHandler(Engine::Input::UIManager &ui)
    : Engine::Input::Handler<GameOverHandler>(ui, "GameOver", Engine::Input::HandlerBase::WidgetType::MODAL_OVERLAY)
    {
    }

    std::string_view GameOverHandler::key() const
    {
        return "GameOverHandler";
    }

    void GameOverHandler::startLifetime()
    {
        HandlerBase::startLifetime();
        if (widget()) {
            mLifetime.attach(widget()->pointerClickEvent().connect(&GameOverHandler::restartGame, this));
            mScoreLabel = widget()->getChildRecursive<Engine::Widgets::Label>("ScoreLabel");
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