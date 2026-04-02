#pragma once

#include "Madgine/widgets/widgethandler.h"

#include "Madgine/render/camera.h"

#include "Madgine/render/scenerenderpass.h"

#include "Madgine/render/rendertarget.h"

#include "Generic/intervalclock.h"

#include "Madgine/behavior/nativebehaviorcollector.h"

#include "Modules/threading/customclock.h"

#include "Madgine/scene/behavior/scenesenders.h"

#include "Generic/manuallifetime.h"

namespace ClickBrick {

    struct GameManager : Engine::Widgets::WidgetHandler<GameManager> {

        GameManager(Engine::Behavior::HandlerManager &ui);

        Engine::Threading::Task<bool> init() override;
        Engine::Threading::Task<void> finalize() override;

        std::string_view key() const override;

        void setWidget(Engine::Widgets::WidgetBase *widget) override;

        Engine::Behavior::Behavior game();

        void spawnBrick();

        void onPointerClickHandler(const Engine::Widgets::PointerClickEvent &evt);

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

        Engine::IntervalClock<Engine::Threading::CustomTimepoint> mSceneClock;
        
        Engine::ManualLifetime<Engine::Render::SceneRenderPass> mSceneRenderer;
        std::unique_ptr<Engine::Render::RenderTarget> mGameRenderTarget;
    };

    Engine::Behavior::Behavior Brick(float speed, Engine::Vector3 dir, Engine::Quaternion q, Engine::Scene::EntityBinding entity = {});
    Engine::Behavior::Behavior Test(Engine::Scene::EntityBinding entity = {});
    Engine::Behavior::Behavior Test2(Engine::Scene::EntityBinding entity = {});

}