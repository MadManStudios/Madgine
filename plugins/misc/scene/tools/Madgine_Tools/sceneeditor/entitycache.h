#pragma once

#include "Generic/flags.h"

#include "Madgine/scene/entity/entityptr.h"

namespace Engine {
namespace Tools {

    FLAGS(EntityCacheSortingFlags,
        GroupByContainer);

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

        void setSortingFlags(EntityCacheSortingFlags flags);
        EntityCacheSortingFlags sortingFlags() const;

    protected:
        bool update(Node &node, const Scene::Entity::EntityPtr &parent = {});
        void createEntityMapping(Scene::Entity::EntityPtr e);

    private:
        EntityCacheSortingFlags mSortingFlags;

        SceneEditor &mEditor;

        std::list<Node> mEntityCache;
        std::map<Scene::Entity::EntityPtr, Node *> mEntityMapping;
    };

}
}