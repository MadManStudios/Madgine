#pragma once

#include "../entitycomponent.h"

#include "Madgine/skeletonloader/skeletonloader.h"

#include "Madgine/render/buffer.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Skeleton : EntityComponent<Skeleton> {

            std::string_view getName() const;
            void setName(std::string_view name);

            const Render::SkeletonDescriptor *data() const;

            void set(Render::SkeletonLoader::Handle handle);

            Render::SkeletonLoader::Resource *get() const;
            const Render::SkeletonLoader::Handle &handle() const;

            void resetMatrices(Matrix4 *matrices);

            Render::GPUBuffer<Matrix4[]> mBoneMatrices;

            std::vector<Matrix4> mLocalMatrices;

        private:
            typename Render::SkeletonLoader::Handle mSkeleton;

            bool mDirty = false;
        };

        using SkeletonPtr = EntityComponentPtr<Skeleton>;

    }
}
}

REGISTER_TYPE(Engine::Scene::Entity::Skeleton)
REGISTER_TYPE(Engine::Scene::Entity::EntityComponentList<Engine::Scene::Entity::Skeleton>)