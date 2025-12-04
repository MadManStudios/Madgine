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

            Matrix4 matrix() const;
            Matrix4 worldMatrix() const;
            Matrix4 parentMatrix() const;

            void setParent(EntityPtr parent);
            const EntityPtr &parent() const;

            Vector3 mPosition;
            Vector3 mScale = Vector3::UNIT_SCALE;
            Quaternion mOrientation;

        private:
            EntityPtr mParent;
        };

    }
}
}