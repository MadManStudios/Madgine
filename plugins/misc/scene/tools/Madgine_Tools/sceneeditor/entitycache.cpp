#include "../scenerenderertoolslib.h"

#include "entitycache.h"


#include "Madgine/scene/scenemanager.h"

#include "sceneeditor.h"

namespace Engine {
namespace Tools {

    EntityCache::EntityCache(SceneEditor &editor)
        : mEditor(editor)
    {
    }

    void EntityCache::eraseNode(const Node &node)
    {
        for (const Node &child : node.mChildren)
            eraseNode(child);
        mEntityMapping.erase(node.mEntity);
    }

    void EntityCache::update()
    {
        // Update + Remove deleted Entities
        mEntityCache.remove_if([this](Node &node) { return update(node); });

        // Add missing Entities
        for (Scene::SceneContainer &container : mEditor.sceneMgr().containers()) {
            for (Scene::Entity::EntityPtr entity : container.entities()) {
                if (!mEntityMapping.count(entity))
                    createEntityMapping(std::move(entity));
            }
        }
    }

    bool EntityCache::update(Node &node, const Scene::Entity::EntityPtr &parent)
    {
        bool alive = Execution::access_binding(node.mEntity, [&](Scene::Entity::Entity &e) {
            return parent == e.parent();
        });
        if (!alive) {
            eraseNode(node);
            return true;
        }
        node.mChildren.remove_if([&](Node &childNode) { return update(childNode, node.mEntity); });
        return false;
    }

    void EntityCache::createEntityMapping(Scene::Entity::EntityPtr e)
    {
        Scene::Entity::EntityPtr parent;

        Execution::access_binding(e, [&](Scene::Entity::Entity &entity) {            
            parent = entity.parent();
        });

        if (parent) {
            if (!mEntityMapping.count(parent))
                createEntityMapping(parent);
        }

        std::list<Node> &container = parent ? mEntityMapping[parent]->mChildren : mEntityCache;

        container.push_back({ std::move(e) });
        mEntityMapping[container.back().mEntity] = &container.back();
    }

    std::list<EntityCache::Node>::const_iterator EntityCache::begin() const
    {
        return mEntityCache.begin();
    }

    std::list<EntityCache::Node>::const_iterator EntityCache::end() const
    {
        return mEntityCache.end();
    }

    void EntityCache::setSortingFlags(EntityCacheSortingFlags flags)
    {
        mSortingFlags = flags;
    }

    EntityCacheSortingFlags EntityCache::sortingFlags() const
    {
        return mSortingFlags;
    }

}
}