#include "clickbricklib.h"

#include "gamemanager.h"

#include "Platform/log/logsenders.h"

#include "Meta/math/boundingbox.h"
#include "Meta/math/geometry3.h"
#include "Meta/math/ray3.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/threading/datamutex.h"

#include "Madgine/app/application.h"
#include "Madgine/behavior/awaitables/awaitablebinding.h"
#include "Madgine/handlermanager.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/scenemainwindowcomponent.h"
#include "Madgine/scene/behavior/scenesenders.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/scenemanager.h"
#include "Madgine/widgets/events.h"
#include "Madgine/widgets/label.h"
#include "Madgine/widgets/scenewindow.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/reflect/metatable_impl.h"

#include "gameoverhandler.h"

UNIQUECOMPONENT(ClickBrick::GameManager)

METATABLE_BEGIN_BASE(ClickBrick::GameManager, Engine::Widgets::WidgetHandlerBase)
    MEMBER(mCamera)
METATABLE_END(ClickBrick::GameManager)

//NATIVE_BEHAVIOR(ClickBrick_Brick, ClickBrick::Brick, Engine::Behavior::InputParameter<"Speed", float>, Engine::Behavior::InputParameter<"Direction", Engine::Vector3>, Engine::Behavior::InputParameter<"Rotation", Engine::Quaternion>)

namespace ClickBrick {

GameManager::GameManager(Engine::Behavior::HandlerManager &ui)
    : Engine::Widgets::WidgetHandler<GameManager>(ui, "Ingame")
    , mSceneMgr(ui.app().getGlobalAPIComponent<Engine::Scene::SceneManager>())
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

    Engine::construct(mSceneRenderer, mSceneMgr, mUI.window().getWindowComponent<Engine::Render::SceneMainWindowComponent>().renderData(), mUI.window().getWindowComponent<Engine::Render::SceneMainWindowComponent>().pointShadowRenderData(), mCamera, 50);

    mGameRenderTarget = mUI.window().getRenderer()->createRenderTexture({ 1, 1 }, { .mName = "Game", .mFormat = Engine::Render::FORMAT_RGBA8_SRGB });
    mGameRenderTarget->addRenderPass(&mSceneRenderer);

    co_return co_await WidgetHandlerBase::init();
}

Engine::Threading::Task<void> GameManager::finalize()
{
    mGameRenderTarget.reset();

    Engine::destruct(mSceneRenderer);

    co_await WidgetHandlerBase::finalize();
}

void GameManager::setWidget(Engine::Widgets::WidgetBase *widget)
{
    WidgetHandlerBase::setWidget(widget);

    if (widget) {
        mGameWindow = widget->getChildRecursive<Engine::Widgets::SceneWindow>("GameView");
        mGameWindow->setRenderSource(mGameRenderTarget.get());

        mLifetime.attach(mGameWindow->pointerClickEvent().connect(&GameManager::onPointerClickHandler, this));
        mGameWindow->setAcceptsPointerEvents(true);

        mScoreLabel = widget->getChildRecursive<Engine::Widgets::Label>("Score");
        mLifeLabel = widget->getChildRecursive<Engine::Widgets::Label>("Life");

    } else {
        mGameWindow = nullptr;
        mScoreLabel = nullptr;
        mLifeLabel = nullptr;
    }
}

Engine::Behavior::Behavior GameManager::game()
{
    std::chrono::microseconds spawnInterval = 1s;
    std::chrono::microseconds acc = 0s;

    while (true) {
        std::chrono::microseconds timeSinceLastFrame = co_await Engine::Scene::yield_simulation();

        acc += timeSinceLastFrame;
        while (acc > spawnInterval) {
            acc -= spawnInterval;
            spawnInterval *= 999;
            spawnInterval /= 1000;
            spawnBrick();
        }
    }
}

void GameManager::spawnBrick()
{
    Engine::Math::Vector3 dir = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
    dir.normalize();
    Engine::Math::Vector3 orientation = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
    Engine::Math::Quaternion q { static_cast<float>(rand()), orientation };

    mUI.app().getGlobalAPIComponent<Engine::Scene::SceneManager>().container("Default").createEntity("", [=](Engine::Scene::Entity::Entity &brick) {
        Engine::Scene::Entity::Transform *t = brick.addComponent<Engine::Scene::Entity::Transform>();
        t->mScale = { 0.01f, 0.01f, 0.01f };
        t->mPosition = dir * -10;        
        t->mOrientation = q;

        brick.addComponent<Engine::Scene::Entity::Mesh>()->setName("Brick");
        brick.getComponent<Engine::Scene::Entity::Mesh>()->handle().info()->setPersistent(true);

        return true; }, [=, this](Engine::Scene::Entity::EntityPtr brick) {
        float speed = rand() / float(RAND_MAX) * 2.0f + 1.0f;
        Engine::Execution::access_binding(brick, [&, this](Engine::Scene::Entity::Entity &e) { e.addBehavior(Brick(speed, dir, q, *this)); });
            mBricks.push_back(brick); });
}

void GameManager::onPointerClickHandler(const Engine::Widgets::PointerClickEvent &evt)
{
    Engine::Math::Ray3 ray = mCamera.mousePointToRay(Engine::Math::Vector2 { static_cast<float>(evt.mWindowPosition.x), static_cast<float>(evt.mWindowPosition.y) }, mGameWindow->getAbsoluteSize().xy());

    Engine::Scene::Entity::EntityPtr hit;
    float distance = std::numeric_limits<float>::max();

    for (const Engine::Scene::Entity::EntityPtr &brick : mBricks) {
        Engine::Execution::access_binding(brick, [&](Engine::Scene::Entity::Entity &e) {
            const Engine::Math::AABB &aabb = e.getComponent<Engine::Scene::Entity::Mesh>()->aabb();
            Engine::Math::BoundingBox bb = e.getComponent<Engine::Scene::Entity::Transform>()->matrix() * aabb;
            if (Engine::Math::UpTo<float, 2> hits = Engine::Math::Intersect(ray, bb)) {
                if (hits[0] < distance) {
                    hit = brick;
                    distance = hits[0];
                }
            }   
        });        
    }

    if (hit) {
        Engine::Execution::access_binding(hit, &Engine::Scene::Entity::Entity::endLifetime);
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
    mSceneMgr.clear();

    mScore = 0;
    mScoreLabel->mText = "Score: " + std::to_string(mScore);
    mLife = 3;
    mLifeLabel->mText = "Life: " + std::to_string(mLife);

    mLifetime.attach(game() | Engine::Behavior::with_named<"Scene">(mSceneMgr));
}

Engine::Behavior::Behavior Brick(float speed, Engine::Math::Vector3 dir, Engine::Math::Quaternion q, GameManager &manager, Engine::Scene::EntityBinding entity)
{

    float qAcc = 1.0f;
    float qSpeed = 1.0f;

    Engine::Math::Quaternion q0 = q;
    Engine::Math::Quaternion q1 = q;

    auto e = co_await *entity;
    Engine::Scene::Entity::Transform *t = e->getComponent<Engine::Scene::Entity::Transform>();

    while (t->mPosition.length() < 10.5f) {

        std::chrono::microseconds elapsedTime = co_await Engine::Scene::yield_simulation();

        float ratio = std::chrono::duration_cast<std::chrono::duration<float>>(elapsedTime).count();

        t->mPosition += speed * ratio * dir;

        qAcc += qSpeed * 0.1f * ratio;

        if (qAcc >= 1.0f) {
            qAcc = 0.0f;
            q0 = q1;

            Engine::Math::Vector3 orientation = { static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2), static_cast<float>(rand() - RAND_MAX / 2) };
            q1 = { static_cast<float>(rand()), orientation };
        }

        t->mOrientation = Engine::Math::slerp(q0, q1, qAcc);
    }

    manager.modLife(-1);

    Engine::Execution::access_binding(*entity, &Engine::Scene::Entity::Entity::endLifetime);

    co_return;
}

}