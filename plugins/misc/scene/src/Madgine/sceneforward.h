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
        struct EntityComponentBase;
        struct EntityComponentListBase;
        struct Transform;
        struct Mesh;
        struct Skeleton;

        struct AnimationState;        

        using EntityPtr = Execution::BindingPtr<Entity&>;
    }

    struct LightManager;
    struct Light;
    
    using EntityBinding = Named<"Entity", Entity::EntityPtr>;
    using SceneBinding = Named<"Scene", SceneManager&>;
}

}
