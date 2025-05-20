#include "../../../scenelib.h"

#include "skeleton.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/skeletonloader/skeletondescriptor.h"

#include "Madgine/resources/resourcemanager.h"

#include "Meta/math/transformation.h"

NAMED_UNIQUECOMPONENT(Skeleton, Engine::Scene::Entity::Skeleton);

METATABLE_BEGIN(Engine::Scene::Entity::Skeleton)
PROPERTY(Skeleton, get, set)
METATABLE_END(Engine::Scene::Entity::Skeleton)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Skeleton)
ENCAPSULATED_FIELD(Skeleton, getName, setName)
SERIALIZETABLE_END(Engine::Scene::Entity::Skeleton)

namespace Engine {
namespace Scene {
    namespace Entity {

        const Render::SkeletonDescriptor *Skeleton::data() const
        {
            return mSkeleton;
        }

        std::string_view Skeleton::getName() const
        {
            return mSkeleton.name();
        }

        void Skeleton::setName(std::string_view name)
        {
            mDirty = true;
            mSkeleton.load(name);
        }

        void Skeleton::set(Render::SkeletonLoader::Handle handle)
        {
            mDirty = true;
            mSkeleton = std::move(handle);
        }

        Render::SkeletonLoader::Resource *Skeleton::get() const
        {
            return mSkeleton.resource();
        }

        const Render::SkeletonLoader::Handle &Skeleton::handle() const
        {
            return mSkeleton;
        }

        void Skeleton::resetMatrices(Matrix4 *matrices)
        {
            assert(mSkeleton.available());

            for (size_t i = 0; i < mSkeleton->mBones.size(); ++i) {
                matrices[i] = mSkeleton->mBones[i].mTTransform.Transpose();
            }
        }
    }
}
}
