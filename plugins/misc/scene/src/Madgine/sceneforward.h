#pragma once

namespace Engine {
namespace Scene {
    struct SceneManager;
    struct SceneContainer;
    struct SceneComponentBase;

    namespace Entity {
        struct EntityPtr;
        template <typename T>
        struct EntityComponentList;
        struct EntityComponentHandle;

        struct Entity;
        struct EntityComponentBase;
        struct EntityComponentListBase;
        struct Transform;
        struct Mesh;
        struct Skeleton;

        struct AnimationState;        
    }

    struct LightManager;
    struct Light;

    using EntityBinding = Binding<"Entity", Entity::Entity*>;
    using SceneBinding = Binding<"Scene", SceneManager*>;
}

}
