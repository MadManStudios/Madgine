#pragma once

namespace Engine {
namespace Scene {
    struct SceneManager;
    struct SceneContainer;
    struct SceneComponentBase;

    namespace Entity {
        struct EntityPtr;
        template <typename T>
        struct EntityComponentPtr;
        template <typename T>
        struct EntityComponentList;
        template <typename T>
        struct EntityComponentOwningHandle;
        template <typename T>
        struct EntityComponentHandle;

        struct Entity;
        struct EntityComponentBase;
        struct EntityComponentListBase;
        struct Transform;
        struct Mesh;
        struct Skeleton;
        using SkeletonPtr = EntityComponentPtr<Skeleton>;

        struct AnimationState;        
    }

    struct LightManager;
    struct Light;

    using EntityBinding = Binding<"Entity", Entity::Entity*>;
    using SceneBinding = Binding<"Scene", SceneManager*>;
}

}
