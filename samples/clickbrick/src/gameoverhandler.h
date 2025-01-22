#pragma once

#include "Madgine/input/widgethandler.h"

namespace ClickBrick {

    struct GameOverHandler : Engine::Input::WidgetHandler<GameOverHandler> {
        SERIALIZABLEUNIT(GameOverHandler)

        GameOverHandler(Engine::Input::UIManager &ui);

        virtual std::string_view key() const override;

        virtual void startLifetime() override;
        virtual void setWidget(Engine::Widgets::WidgetBase *widget) override;

        void restartGame();

        void setScore(int score);

    private:
        Engine::Widgets::Label *mScoreLabel = nullptr;
    };

}

REGISTER_TYPE(ClickBrick::GameOverHandler)