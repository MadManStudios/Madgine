#pragma once

#include "Generic/context.h"

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

            Math::Matrix4 matrix() const;
            Math::Matrix4 worldMatrix(Contextual<Entity &> entity) const;
            Math::Matrix4 parentMatrix(Contextual<Entity &> entity) const;

            Math::Vector3 worldPosition(Contextual<Entity &> entity) const;
            void setWorldPosition(const Math::Vector3 &position, Contextual<Entity &> entity);
            Math::Quaternion worldOrientation(Contextual<Entity &> entity) const;

            Math::Vector3 mPosition;
            Math::Vector3 mScale = Math::Vector3::UNIT_SCALE;
            Math::Quaternion mOrientation;
        };

    }
}
}