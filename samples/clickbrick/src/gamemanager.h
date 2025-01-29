#pragma once

#include "Madgine/widgets/widgethandler.h"

#include "Madgine/render/camera.h"

#include "Madgine/render/scenerenderpass.h"

#include "Madgine/render/rendertarget.h"

#include "Generic/intervalclock.h"

#include "Madgine/nativebehaviorcollector.h"

#include "Modules/threading/customclock.h"

namespace ClickBrick {

    struct GameManager : Engine::Widgets::WidgetHandler<GameManager> {

        GameManager(Engine::HandlerManager &ui);

        virtual Engine::Threading::Task<bool> init() override;
        virtual Engine::Threading::Task<void> finalize() override;

        virtual std::string_view key() const override;

        virtual void setWidget(Engine::Widgets::WidgetBase *widget) override;

        Engine::Threading::Task<void> updateApp();

        void spawnBrick();

        void onPointerClick(const Engine::Input::PointerEventArgs &evt) override;

        void modScore(int diff);
        void modLife(int diff);

        void start();

        Engine::Render::Camera mCamera;

    private:
        Engine::Widgets::SceneWindow *mGameWindow = nullptr;
        Engine::Widgets::Label *mScoreLabel = nullptr;
        Engine::Widgets::Label *mLifeLabel = nullptr;

        int mScore = 0;
        int mLife = 100000;

        Engine::Scene::SceneManager &mSceneMgr;

        std::chrono::microseconds mSpawnInterval = 1s;
        std::chrono::microseconds mAcc = 0s;
        Engine::IntervalClock<Engine::Threading::CustomTimepoint> mSceneClock;
        
        Engine::Render::SceneRenderPass mSceneRenderer;
        std::unique_ptr<Engine::Render::RenderTarget> mGameRenderTarget;
    };

    Engine::Behavior Brick(float speed, Engine::Vector3 dir, Engine::Quaternion q, Engine::Scene::EntityBinding entity = {});
    Engine::Behavior Test(Engine::Scene::EntityBinding entity = {});

}

NATIVE_BEHAVIOR_DECLARATION(ClickBrick_Test)
NATIVE_BEHAVIOR_DECLARATION(ClickBrick_Brick)

REGISTER_TYPE(ClickBrick::GameManager)