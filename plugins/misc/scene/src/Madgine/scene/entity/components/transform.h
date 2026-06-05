#pragma once

#include "Meta/math/matrix4.h"
#include "Meta/math/quaternion.h"
#include "Meta/math/vector3.h"

#include "../entitycomponent.h"
#include "../entityptr.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT Transform : EntityComponent<Transform> {

            SERIALIZABLEUNIT(Transform)

            using EntityComponent<Transform>::EntityComponent;

            Transform(Transform &&) = default;

            Transform &operator=(Transform &&) = default;

            Math::Matrix4 matrix() const;
            Math::Matrix4 worldMatrix() const;
            Math::Matrix4 parentMatrix() const;

            void setParent(EntityPtr parent);
            const EntityPtr &parent() const;

            Math::Vector3 mPosition;
            Math::Vector3 mScale = Math::Vector3::UNIT_SCALE;
            Math::Quaternion mOrientation;

        private:
            EntityPtr mParent;
        };

    }
}
}