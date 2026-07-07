#pragma once

#include "Meta/math/matrix4.h"
#include "Meta/math/ray3.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/scenerenderpass.h"
#include "Madgine/scene/entity/entityptr.h"

#include "Madgine_Tools/im3d/im3drenderpass.h"
#include "gridpass.h"
#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

    struct SceneView {

        SceneView(SceneEditor &editor, int index, Render::SceneRenderData &renderData, Render::PointShadowRenderData &pointShadowRenderData, Im3D::Im3DContext *im3dContext);
        SceneView(SceneView &&);
        ~SceneView();

        bool render();

        Render::Camera &camera();
        Render::SceneRenderPass &sceneRenderer();

    private:
        Render::Camera mCamera;
        std::unique_ptr<Render::RenderTarget> mRenderTarget;
        std::unique_ptr<Render::RenderTarget> mRenderTargetSampled;

        SceneEditor &mEditor;

        IndexType<uint8_t> mDraggedAxis;
        Math::Ray3 mDragStartRay;
        Scene::Entity::EntityPtr mDragEntity;
        Math::Matrix4 mDragStoredMatrix;
        Math::Vector3 mDragStoredPosition;
        Math::Vector3 mDragRelMousePosition;

        bool mAxisDragging = false;

        Render::SceneRenderPass mSceneRenderer;
        GridPass mGridRenderer;
        Render::Im3DRenderPass mIm3DRenderer;

        int mIndex;
    };

}
}