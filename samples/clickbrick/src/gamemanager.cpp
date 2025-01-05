#include "clickbricklib.h"

#include "gamemanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/scene/scenemanager.h"

#include "Madgine/widgets/scenewindow.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/transform.h"

#include "Meta/math/geometry3.h"

#include "Meta/math/boundingbox.h"

#include "Madgine/widgets/label.h"

#include "gameoverhandler.h"

#include "Madgine/app/application.h"

#include "Meta/math/ray3.h"

#include "Madgine/scene/entity/entity.h"

#include "Modules/threading/datamutex.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine/input/uimanager.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/render/scenemainwindowcomponent.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Madgine/render/rendercontext.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/awaitables/awaitablesender.h"

#include "Madgine/scene/behavior/scenesenders.h"

UNIQUECOMPONENT(ClickBrick::GameManager)

METATABLE_BEGIN_BASE(ClickBrick::GameManager, Engine::Input::HandlerBase)
MEMBER(mCamera)
METATABLE_END(ClickBrick::GameManager)

NATIVE_BEHAVIOR(ClickBrick_Test, ClickBrick::Test)
NATIVE_BEHAVIOR(ClickBrick_Brick, ClickBrick::Brick, Engine::InputParameter<float>, Engine::InputParameter<Engine::Vector3>, Engine::InputParameter<Engine::Quaternion>)

namespace ClickBrick {

GameManager::GameManager(Engine::Input::UIManager &ui)
    : Engine::Input::Handler<GameManager>(ui, "GameView")
    , mSceneMgr(ui.app().getGlobalAPIComponent<Engine::Scene::SceneManager>())
    , mSceneRenderer(ui.window().getWindowComponent<Engine::Render::SceneMainWindowComponent>(), &mCamera, 50)
    , mSceneClock(mSceneMgr.clock().now())
{
}

std::string_view GameManager::key() const
{
    return "GameManager";
}

Engine::Threading::Task<bool> GameManager::init()
{
    mCamera.mPosition = { 0, 0, -10 };
    mCamera.mOrientation = {};

    mGameRenderTarget = mUI.window().getRenderer()->createRenderTexture({ 1, 1 }, { .mName = "Game", .mFormat = Engine::Render::FORMAT_RGBA8_SRGB });
    mGameRenderTarget->addRenderPass(&mSceneRenderer);

    mUI.app().taskQueue()->queueTask(updateApp());

    co_return co_await HandlerBase::init();
}

Engine::Threading::Task<void> GameManager::finalize()
{
    mGameRenderTarget.reset();

    co_await HandlerBase::finalize();
}

void GameManager::startLifetime()
{
    HandlerBase::startLifetime();

    if (widget()) {
        mGameWindow = widget()->getChildRecursive<Engine::Widgets::SceneWindow>("GameView");
        mGameWindow->setRenderSource(mGameRenderTarget.get());

        mScoreLabel = widget()->getChildRecursive<Engine::Widgets::Label>("Score");
        mLifeLabel = widget()->getChildRecursive<Engine::Widgets::Label>("Life");

    } else {
        mGameWindow = nullptr;
        mScoreLabel = nullptr;
        mLifeLabel = nullptr;
    }
}

Engine::Threading::Task<void> GameManager::updateApp()
{
    /* while (mUI.app().taskQueue()->running()) {
        co_await Engine::Threading::TaskQualifiers { mSceneMgr.clock()(1ms) };

        std::chrono::microseconds timeSinceLastFrame = mSceneClock.tick(mSceneMgr.clock().now());

        mAcc += timeSinceLastFrame;
        while (mAcc > mSpawnInterval) {
            mAcc -= mSpawnInterval;
            mSpawnInterval *= 999;
            mSpawnInterval /= 1000;
            co_await mSceneMgr.mutex().locked(Engine::AccessMode::WRITE, [this]() {
                spawnBrick();
            });
        }
    }*/
    co_return;
}

void GameManager::spawnBrick()
{
    Engine::Scene::Entity::EntityPtr brick = mUI.app().getGlobalAPIComponent<Engine::Scene::SceneManager>().container("Default").createEntity();

    Engine::Scene::Entity::Transform *t = brick->addComponent<Engine::Scene::Entity::Transform>().get();
    t->mScale = { 0.01f, 0.01f, 0.01f };

    Engine::Vector3 dir = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
    dir.normalize();
    t->mPosition = dir * -10;

    Engine::Vector3 orientation = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
    Engine::Quaternion q { static_cast<float>(rand()), orientation };
    t->mOrientation = q;

    brick->addComponent<Engine::Scene::Entity::Mesh>().get()->setName("Brick");
    brick->getComponent<Engine::Scene::Entity::Mesh>()->handle().info()->setPersistent(true);

    float speed = rand() / float(RAND_MAX) * 2.0f + 1.0f;

    brick->addBehavior(Brick(speed, dir, q));
}

void GameManager::onPointerClick(const Engine::Input::PointerEventArgs &evt)
{
    Engine::Ray3 ray = mCamera.mousePointToRay(Engine::Vector2 { static_cast<float>(evt.windowPosition.x), static_cast<float>(evt.windowPosition.y) }, mGameWindow->getAbsoluteSize().xy());

    Engine::Scene::Entity::EntityPtr hit;
    float distance = std::numeric_limits<float>::max();

    /* for (const Engine::Scene::Entity::EntityPtr &e : mBricks) {
        const Engine::AABB &aabb = e->getComponent<Engine::Scene::Entity::Mesh>()->aabb();
        Engine::BoundingBox bb = e->getComponent<Engine::Scene::Entity::Transform>()->matrix() * aabb;
        if (Engine::UpTo<float, 2> hits = Engine::Intersect(ray, bb)) {
            if (hits[0] < distance) {
                hit = e;
                distance = hits[0];
            }
        }
    }*/

    if (hit) {
        hit->endLifetime();
        modScore(1);
    }
}

void GameManager::modScore(int diff)
{
    mScore += diff;
    mScoreLabel->mText = "Score: " + std::to_string(mScore);
}

void GameManager::modLife(int diff)
{
    mLife += diff;
    mLifeLabel->mText = "Life: " + std::to_string(mLife);

    if (mLife <= 0) {
        getHandler<GameOverHandler>().setScore(mScore);
        getHandler<GameOverHandler>().open();
    }
}

void GameManager::start()
{
    mSpawnInterval = 1s;
    mAcc = 0us;

    mScore = 0;
    mScoreLabel->mText = "Score: " + std::to_string(mScore);
    mLife = 3;
    mLifeLabel->mText = "Life: " + std::to_string(mLife);
}

Engine::Behavior Brick(float speed, Engine::Vector3 dir, Engine::Quaternion q, Engine::Scene::EntityBinding entity)
{

    Engine::Scene::Entity::Entity *e = co_await entity;

    float qAcc = 1.0f;
    float qSpeed = 1.0f;

    Engine::Quaternion q0 = q;
    Engine::Quaternion q1 = q;

    bool loop = true;
    while (loop) {

        std::chrono::microseconds elapsedTime = co_await Engine::Scene::yield_simulation();

        Engine::Scene::Entity::Transform *t = e->getComponent<Engine::Scene::Entity::Transform>();

        float ratio = std::chrono::duration_cast<std::chrono::duration<float>>(elapsedTime).count();

        t->mPosition += speed * ratio * dir;

        qAcc += qSpeed * 0.1f * ratio;

        if (qAcc >= 1.0f) {
            qAcc = 0.0f;
            q0 = q1;

            Engine::Vector3 orientation = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
            q1 = { static_cast<float>(rand()), orientation };
        }

        t->mOrientation = Engine::slerp(q0, q1, qAcc);

        loop = t->mPosition.length() < 10.5f;
    }

    e->endLifetime();

    co_return;
}

Engine::Behavior Test(Engine::Scene::EntityBinding entity)
{

    Engine::Scene::Entity::Entity *e = co_await entity;

    bool loop = true;
    while (loop) {

        co_await Engine::Execution::sequence(
            Engine::Scene::wait_simulation(1s),
            Engine::Scene::wait_simulation(2s),
            Engine::Scene::wait_simulation(3s),
            Engine::Scene::wait_simulation(4s));

        /* Engine::Scene::Entity::Transform *t = e->getComponent<Engine::Scene::Entity::Transform>();

        float ratio = std::chrono::duration_cast<std::chrono::duration<float>>(elapsedTime).count();

        t->mPosition += speed * ratio * dir;

        qAcc += qSpeed * 0.1f * ratio;

        if (qAcc >= 1.0f) {
            qAcc = 0.0f;
            q0 = q1;

            Engine::Vector3 orientation = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
            q1 = { static_cast<float>(rand()), orientation };
        }

        t->mOrientation = Engine::slerp(q0, q1, qAcc);

        loop = t->mPosition.length() < 10.5f;*/
    }

    co_return;
}

}
