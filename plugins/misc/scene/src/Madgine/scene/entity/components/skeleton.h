#pragma once

#include "Madgine/render/ptr.h"
#include "Madgine/skeletonloader/skeletonloader.h"

#include "../entitycomponent.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Skeleton : EntityComponent<Skeleton> {

            using EntityComponent<Skeleton>::EntityComponent;

            std::string_view getName() const;
            void setName(std::string_view name);

            const Render::SkeletonDescriptor *data() const;

            void set(Render::SkeletonLoader::Handle handle);

            Render::SkeletonLoader::Resource *get() const;
            const Render::SkeletonLoader::Handle &handle() const;

            void resetMatrices(Math::Matrix4 *matrices);

            Render::GPUPtr<Math::Matrix4[]> mBoneMatrices;

            std::vector<Math::Matrix4> mLocalMatrices;

        private:
            typename Render::SkeletonLoader::Handle mSkeleton;

            bool mDirty = false;
        };

    }
}
}