#pragma once

#include "Generic/execution/binding.h"

#include "Madgine/scene/entity/entityptr.h"

namespace Engine {
namespace Tools {

    struct EntityCache {

        EntityCache(SceneEditor &editor);

        // Entity-Cache
        struct Node {
            Scene::Entity::EntityPtr mEntity;
            std::list<Node> mChildren;
        };

        void update();
        void eraseNode(const Node &node);

        std::list<Node>::const_iterator begin() const;
        std::list<Node>::const_iterator end() const;

    protected:
        bool update(Node &node, const Scene::Entity::EntityPtr &parent = {});
        void createEntityMapping(Scene::Entity::EntityPtr e);
        

    private:
        SceneEditor &mEditor;

        std::list<Node> mEntityCache;
        std::map<Scene::Entity::EntityPtr, Node *> mEntityMapping;
    };

}
}