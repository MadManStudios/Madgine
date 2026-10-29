#include "../scenerendererlib.h"

#include "camerascenerenderdata.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/scene/entity/components/material.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/scenemanager.h"
#include "Madgine/window/mainwindow.h"

#include "frustumculling.h"
#include "scenemainwindowcomponent.h"

namespace Engine {
namespace Render {

    CameraSceneRenderData::CameraSceneRenderData(SceneRenderData &renderData, Camera &camera)
        : mCamera(camera)
        , mRenderData(renderData)
    {
    }

    CameraSceneRenderData::~CameraSceneRenderData()
    {
        assert(mRefCount == 0);
    }

    Threading::ImmediateTask<RenderFuture> CameraSceneRenderData::render(RenderContext *context)
    {

        co_await mRenderData.update(context);

        for (auto &[key, transforms] : mInstances)
            transforms.clear();

        mViewMatrix = mCamera.getViewMatrix();

        FrustumCuller culler { mCamera.getFrustum(mMaxAspectRatio) };

        mMaxAspectRatio = 0.0f;

        for (const EntityRenderData &entity : mRenderData.data()) {

            if (!entity.mMesh)
                continue;

            const Math::AABB &aabb = entity.mMesh->mAABB;

            if (entity.mBones || culler.visible(aabb, entity.mTransformMatrix))
                mInstances[NonInstancedData { entity.mMesh, entity.mResource, entity.mDiffuseColor.w < 1.0f }].push_back(&entity);
        }

        for (auto it = mInstances.begin(); it != mInstances.end();) {
            if (it->second.empty()) {
                it = mInstances.erase(it);
            } else {
                ++it;
            }
        }

        co_return {};
    }

    void CameraSceneRenderData::unregister()
    {
        if (--mRefCount == 0)
            mRenderData.unregisterCamera(mCamera);
    }

    const std::map<CameraSceneRenderData::NonInstancedData, std::vector<const EntityRenderData *>> &CameraSceneRenderData::get(float aspectRatio)
    {
        mMaxAspectRatio = std::max(mMaxAspectRatio, aspectRatio);
        return mInstances;
    }

    const Math::Matrix4 &CameraSceneRenderData::getViewMatrix() const
    {
        return mViewMatrix;
    }

}
}
