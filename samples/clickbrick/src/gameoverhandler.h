#pragma once

#include "Madgine/widgets/widgethandler.h"

namespace ClickBrick {

    struct GameOverHandler : Engine::Widgets::WidgetHandler<GameOverHandler> {
        SERIALIZABLEUNIT(GameOverHandler)

        GameOverHandler(Engine::HandlerManager &ui);

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