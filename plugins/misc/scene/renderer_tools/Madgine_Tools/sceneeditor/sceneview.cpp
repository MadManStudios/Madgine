#include "../scenerenderertoolslib.h"

#include "sceneview.h"

#include "imgui/imgui.h"

#include "imgui/imguiaddons.h"

#include "Madgine/render/rendercontext.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "im3d/im3d.h"

#include "sceneeditor.h"

#include "Madgine/scene/entity/components/transform.h"

#include "Meta/math/plane.h"

#include "Meta/math/geometry3.h"

#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/entity.h"

#include "Madgine/scene/entity/components/mesh.h"

#include "Madgine/render/texture.h"

#include "Meta/math/transformation.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine_Tools/interactivecamera.h"

#include "Madgine_Tools/imgui/clientimroot.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/render/scenemainwindowcomponent.h"

namespace Engine {
namespace Tools {

    static Plane cameraPlane(const Render::Camera &camera, const Vector3 &point, const Vector3 *axis1 = nullptr, const Vector3 *axis2 = nullptr)
    {

        const Ray3 &ray = camera.toRay();

        Vector3 normal;

        if (axis1) {
            Vector3 helper;
            if (axis2) {
                helper = *axis2;
            } else {
                helper = axis1->crossProduct(ray.mDir);
            }
            normal = helper.crossProduct(*axis1);
        } else {
            normal = ray.mDir;
        }

        return { point, normal };
    }

    SceneView::SceneView(SceneEditor *editor, Engine::Render::RenderContext *context)
        : mEditor(editor)
        , mSceneRenderer(static_cast<ClientImRoot&>(editor->root()).window().getWindowComponent<Render::SceneMainWindowComponent>(), &mCamera, 25)
        , mGridRenderer(&mCamera, 50)
        , mIm3DRenderer(&mCamera, 75)
        , mIndex(editor->createViewIndex())
    {
        mCamera.mPosition = { 0, 0.5, -1 };

        mRenderTargetSampled = context->createRenderTexture({ 1000, 1000 }, { .mName = "SceneView 4x", .mType = Render::TextureType_2DMultiSample, .mFormat = Render::FORMAT_RGBA8_SRGB, .mSamples = 4 });

        mRenderTargetSampled->addRenderPass(&mSceneRenderer);

        mRenderTargetSampled->addRenderPass(&mGridRenderer);

        mRenderTargetSampled->addRenderPass(&mIm3DRenderer);

        mRenderTarget = context->createRenderTexture({ 1000, 1000 }, { .mName = "SceneView", .mFormat = Render::FORMAT_RGBA8_SRGB, .mBlitSource = mRenderTargetSampled.get() });

        static_cast<ClientImRoot &>(mEditor->root()).addRenderTarget(mRenderTarget.get());
    }

    SceneView::SceneView(SceneView &&) = default;

    SceneView::~SceneView()
    {
        static_cast<ClientImRoot &>(mEditor->root()).removeRenderTarget(mRenderTarget.get());

        mRenderTargetSampled->removeRenderPass(&mIm3DRenderer);

        mRenderTargetSampled->removeRenderPass(&mGridRenderer);

        mRenderTargetSampled->removeRenderPass(&mSceneRenderer);
    }

    bool SceneView::render()
    {
        bool open = true;

        ImGui::SetNextWindowDockID(mEditor->root().dockSpaceId(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints({ 100, 100 }, { 1000000, 1000000 });
        if (ImGui::Begin(("SceneView##SceneView" + std::to_string(mIndex)).c_str(), &open)) {

            constexpr Vector3 axes[3] = {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 }
            };

            ImGuiIO &io = ImGui::GetIO();
            Im3DIO &io3D = Im3D::GetIO();

            const Ray3 &ray = Im3D::GetMouseRay();

            ImVec2 region = ImGui::GetContentRegionAvail();
            Vector2i iRegion { static_cast<int>(region.x), static_cast<int>(region.y) };
            if (iRegion.x > 0 && iRegion.y > 0)
                mRenderTarget->resize(iRegion);
            bool pressed = ImGui::ImageButton((void *)mRenderTarget->texture()->resource().mPtr, region, { 0, 0 }, { 1, 1 }, 0);
            if (pressed && !mState.mDragging[0])
                if (!Im3D::IsAnyObjectHovered())
                    mEditor->deselect();

            if (ImGui::InteractiveView(mState)) {

                if (io.MouseClicked[0]) {
                    if (mEditor->hoveredAxis() >= 0) {
                        mDraggedAxis = mEditor->hoveredAxis();
                        mDragStartRay = ray;
                        mDragStoredPosition = mEditor->hoveredTransform()->mPosition;

                        Vector3 axis = axes[mDraggedAxis];

                        Plane plane = cameraPlane(mCamera, mDragStoredPosition, &axis);

                        if (auto intersection = Intersect(mDragStartRay, plane)) {
                            mDragTransform = mEditor->hoveredTransform();
                            mDragStoredMatrix = mEditor->hoveredTransform()->matrix();
                            mDragRelMousePosition = mDragStartRay.point(intersection[0]) - mDragStoredPosition;
                            mAxisDragging = true;
                        } else {
                            mAxisDragging = false;
                        }
                    } else {
                        mAxisDragging = false;
                    }
                }

                Vector2 pos = ImGui::GetItemRectMin();
                Vector2 size = ImGui::GetItemRectSize();

                io3D.mNextFrameMouseRay = mCamera.mousePointToRay(Vector2 { io.MousePos } - pos, size);
            }

            InteractiveCamera(mState, mCamera);

            if (mState.mDragging[0] && mAxisDragging) {

                Vector3 axis = axes[mDraggedAxis];

                Plane targetPlane = cameraPlane(mCamera, mDragStoredPosition, &axis);

                if (auto intersection = Intersect(ray, targetPlane)) {

                    Vector3 distance = ray.point(intersection[0]) - mDragStoredPosition - mDragRelMousePosition;

                    if (mDraggedAxis != 0)
                        distance.x = 0.0f;
                    if (mDraggedAxis != 1)
                        distance.y = 0.0f;
                    if (mDraggedAxis != 2)
                        distance.z = 0.0f;

                    mDragTransform->mPosition = mDragStoredPosition + mDragTransform->parentMatrix().ToMat3().Inverse() * distance;
                }
            }

            if (ImGui::BeginDragDropTarget()) {
                Vector3 pos = ray.point(5.0f);
                Render::GPUMeshLoader::Resource *resource;
                if (ImGui::AcceptDraggableValueType(resource)) {
                    Scene::Entity::EntityPtr e = mEditor->sceneMgr().container("Default").createEntity();
                    e->addComponent<Scene::Entity::Transform>().get()->mPosition = pos;
                    e->addComponent<Scene::Entity::Mesh>().get()->set(resource);
                    mEditor->select(e);
                } else if (ImGui::IsDraggableValueTypeBeingAccepted(resource)) {
                    Render::GPUMeshLoader::Handle handle = resource->loadData();
                    handle.info()->setPersistent(true);
                    Im3D::NativeMesh(handle->mMaterials.front().mResourceBlock, handle->mAABB, TranslationMatrix(pos));
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();

        return open;
    }

    Render::Camera &SceneView::camera()
    {
        return mCamera;
    }

    Render::SceneRenderPass &SceneView::sceneRenderer()
    {
        return mSceneRenderer;
    }

}
}

METATABLE_BEGIN(Engine::Tools::SceneView)
READONLY_PROPERTY(Camera, camera)
READONLY_PROPERTY(SceneRenderer, sceneRenderer)
METATABLE_END(Engine::Tools::SceneView)