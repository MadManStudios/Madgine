#pragma once

namespace Engine {
namespace Scene {
    struct SceneManager;
    struct SceneContainer;
    struct SceneComponentBase;

    namespace Entity {
        template <typename T>
        struct EntityComponentList;
        struct EntityComponentHandle;

        struct Entity;
        struct EntityHandle;
        struct EntityComponentBase;
        struct EntityComponentListBase;
        struct Transform;
        struct Mesh;
        struct Skeleton;

        struct AnimationState;

        struct EntityPtr;
    }

    struct LightManager;
    struct Light;

    using EntityBinding = Behavior::Named<"Entity", Entity::EntityPtr>;
    using SceneBinding = Behavior::Named<"Scene", SceneManager &>;
}

}
