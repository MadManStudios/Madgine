#include "../bullettoolslib.h"

#include "bulletdebug.h"

#include "imgui/imgui.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "im3d/im3d.h"

#include "Madgine/app/application.h"
#include "Madgine/physics/physicsmanager.h"
#include "Madgine/scene/scenemanager.h"

#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

#include "Meta/math/color4.h"

namespace Engine {
namespace Tools {

    BulletDebug::BulletDebug(ImRoot &root)
        : Tool<BulletDebug>(root)
    {
    }

    Threading::Task<bool> BulletDebug::init()
    {
        mManager = &App::Application::getSingleton().getGlobalAPIComponent<Scene::SceneManager>().getComponent<Physics::PhysicsManager>();

        co_await mManager->state();

        mManager->world().setDebugDrawer(this);

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> BulletDebug::finalize()
    {

        co_await ToolBase::finalize();
    }

    void BulletDebug::render()
    {
        if (ImGui::Begin("BulletDebug-Config", &mVisible)) {
            ImGui::CheckboxFlags("DBG_DrawWireframe", &mDebugMode, DBG_DrawWireframe);
            ImGui::CheckboxFlags("DBG_DrawAabb", &mDebugMode, DBG_DrawAabb);
            ImGui::CheckboxFlags("DBG_DrawFeaturesText", &mDebugMode, DBG_DrawFeaturesText);
            ImGui::CheckboxFlags("DBG_DrawContactPoints", &mDebugMode, DBG_DrawContactPoints);
            ImGui::CheckboxFlags("DBG_NoDeactivation", &mDebugMode, DBG_NoDeactivation);
            ImGui::CheckboxFlags("DBG_NoHelpText", &mDebugMode, DBG_NoHelpText);
            ImGui::CheckboxFlags("DBG_DrawText", &mDebugMode, DBG_DrawText);
            ImGui::CheckboxFlags("DBG_ProfileTimings", &mDebugMode, DBG_ProfileTimings);
            ImGui::CheckboxFlags("DBG_EnableSatComparison", &mDebugMode, DBG_EnableSatComparison);
            ImGui::CheckboxFlags("DBG_DisableBulletLCP", &mDebugMode, DBG_DisableBulletLCP);
            ImGui::CheckboxFlags("DBG_EnableCCD", &mDebugMode, DBG_EnableCCD);
            ImGui::CheckboxFlags("DBG_DrawConstraints", &mDebugMode, DBG_DrawConstraints);
            ImGui::CheckboxFlags("DBG_DrawConstraintLimits", &mDebugMode, DBG_DrawConstraintLimits);
            ImGui::CheckboxFlags("DBG_FastWireframe", &mDebugMode, DBG_FastWireframe);
            ImGui::CheckboxFlags("DBG_DrawNormals", &mDebugMode, DBG_DrawNormals);
            ImGui::CheckboxFlags("DBG_DrawFrames", &mDebugMode, DBG_DrawFrames);
        }
        ImGui::End();
    }

    void BulletDebug::renderMenu()
    {
        ToolBase::renderMenu();
        if (mVisible) {

            if (ImGui::BeginMenu("BulletDebug")) {

                ImGui::EndMenu();
            }

            auto guard = mManager->sceneMgr().mutex().lock(AccessMode::READ);
            mManager->world().debugDrawWorld();
        }
    }

    std::string_view BulletDebug::key() const
    {
        return "BulletDebug";
    }

    void BulletDebug::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor)
    {
        Im3D::Line(Vector3 { from }, Vector3 { to }, { .mColorA = { Color3 { fromColor }, 1 }, .mColorB = { Color3 { toColor }, 1 } });
    }

    void BulletDebug::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
    {
        Im3D::Line(Vector3 { from }, Vector3 { to }, { .mColorA = { Color3 { color }, 1 } });
    }

    void BulletDebug::drawSphere(const btVector3 &p, btScalar radius, const btVector3 &color)
    {
        throw 0;
    }

    void BulletDebug::drawTriangle(const btVector3 &a, const btVector3 &b, const btVector3 &c, const btVector3 &color, btScalar alpha)
    {
        throw 0;
    }

    void BulletDebug::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
    {
        Im3D::Sphere(Vector3 { PointOnB }, distance + 0.01f, { .mColor = { Color3 { color }, 1 }, .mDetail = 0 });
    }

    void BulletDebug::reportErrorWarning(const char *warningString)
    {
        LOG_WARNING(warningString);
    }

    void BulletDebug::draw3dText(const btVector3 &location, const char *textString)
    {
        throw 0;
    }

    void BulletDebug::setDebugMode(int debugMode)
    {
        mDebugMode = debugMode;
    }

}
}

UNIQUECOMPONENT(Engine::Tools::BulletDebug);

METATABLE_BEGIN_BASE(Engine::Tools::BulletDebug, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::BulletDebug)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::BulletDebug, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::BulletDebug)
