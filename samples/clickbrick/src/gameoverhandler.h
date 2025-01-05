#pragma once

#include "Madgine/input/handler.h"

namespace ClickBrick {

    struct GameOverHandler : Engine::Input::Handler<GameOverHandler> {
        SERIALIZABLEUNIT(GameOverHandler)

        GameOverHandler(Engine::Input::UIManager &ui);

        virtual std::string_view key() const override;

        virtual void startLifetime() override;

        void restartGame();

        void setScore(int score);

    private:
        Engine::Widgets::Label *mScoreLabel = nullptr;
    };

}

REGISTER_TYPE(ClickBrick::GameOverHandler)