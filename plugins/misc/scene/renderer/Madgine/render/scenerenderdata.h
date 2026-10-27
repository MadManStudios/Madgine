#pragma once

#include "Modules/threading/customclock.h"

#include "Madgine/meshloader/gpumeshdata.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/render/renderdata.h"
#include "Madgine/render/resourceblock.h"
#include "Madgine/scene/entity/components/transform.h"

namespace Engine {
namespace Render {

    struct EntityRenderData {

        void ensureAlive(Scene::SceneManager &scene);
        void markDead();
        bool alive() const;

        Scene::Entity::Transform mPreviousTransform;

        Math::Matrix4 mTransformMatrix;

        size_t mLastRenderedFrame = 0;
        Threading::CustomTimepoint mLastUpdate = std::chrono::steady_clock::time_point::min();

        GPUMeshLoader::Handle mMesh;
        bool mCastShadows = true;
        GPUPtr<Math::Matrix4[]> mBones;
        ResourceBlock mResource;
        Math::Vector4 mDiffuseColor { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Vector3 mEmissiveColor = Math::Vector3::ZERO;

        // Cached Parent
        const Scene::Entity::Transform* mParentTransform = nullptr;
        EntityRenderData* mParentData = nullptr;
    };

    struct MADGINE_SCENE_RENDERER_EXPORT SceneRenderData : RenderData {

        SceneRenderData(Scene::SceneManager &scene);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        void setup(RenderContext *context);
        void shutdown(RenderContext *context);

        ResourceBlock defaultMaterial();

        decltype(auto) data() const
        {
            return mData | std::views::filter(&EntityRenderData::alive);
        }

    protected:
        Threading::TaskQueue *queue(RenderContext *) override;

        void interpolateTransform(EntityRenderData &d, Scene::Entity::Transform &t, Scene::Entity::Entity &e);

    private:
        Scene::SceneManager &mScene;

        UniqueResourceBlock mDefaultMaterial;

        std::deque<EntityRenderData> mData;
    };

}
}