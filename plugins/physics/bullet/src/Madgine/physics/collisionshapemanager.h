#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

class btCollisionShape;

namespace Engine {
namespace Physics {

    struct CollisionShapeInstance;

    struct CollisionShape;

    struct CollisionShapeInstanceDeleter {
        void operator()(CollisionShapeInstance *);
    };

    using CollisionShapeInstancePtr = std::unique_ptr<CollisionShapeInstance, CollisionShapeInstanceDeleter>;

    struct CollisionShapeManager : Resources::ResourceLoader<CollisionShapeManager, std::unique_ptr<CollisionShape>> {

        struct InstanceHandle {

            InstanceHandle() = default;
            InstanceHandle(Handle shape);
            InstanceHandle(Resource *res);
            InstanceHandle(const InstanceHandle &);
            InstanceHandle(InstanceHandle &&other) noexcept = default;

            InstanceHandle &operator=(const InstanceHandle &other);
            InstanceHandle &operator=(InstanceHandle &&other);

            void load(std::string_view name, CollisionShapeManager *loader = &CollisionShapeManager::getSingleton());
            void reset();

            Resource *resource() const;

            CollisionShapeInstance *operator->() const;

            operator CollisionShapeInstance *() const;

            friend struct Serialize::Operations<InstanceHandle>;

            friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, InstanceHandle& handle, Serialize::FormattedSerializeStream&, bool success, const CallerHierarchyBasePtr&) {
                return {};
            }

        private:
            CollisionShapeInstancePtr mInstance;
        };

        CollisionShapeManager();

        Threading::ImmediateTask<bool> loadImpl(std::unique_ptr<CollisionShape> &shape, ResourceDataInfo &info);
        void unloadImpl(std::unique_ptr<CollisionShape> &shape);
    };

    struct CollisionShape {
        virtual ~CollisionShape() = default;
        virtual CollisionShapeInstancePtr create(typename CollisionShapeManager::Handle shape) = 0;
    };

    struct CollisionShapeInstance : Serialize::VirtualSerializableUnitBase<VirtualScopeBase<>, Serialize::SerializableUnitBase> {
        CollisionShapeInstance(typename CollisionShapeManager::Handle shape = {});
        virtual ~CollisionShapeInstance();
        virtual btCollisionShape *get() = 0;
        virtual void destroy() = 0;
        virtual CollisionShapeInstancePtr clone() = 0;

        CollisionShapeManager::Resource *resource() const;

        bool available() const;

    protected:
        typename CollisionShapeManager::Handle mHandle;
    };

}
namespace Serialize {

    template <>
    struct Operations<Physics::CollisionShapeManager::InstanceHandle> {
        static StreamResult read(FormattedSerializeStream &in, Physics::CollisionShapeManager::InstanceHandle &handle, const char *name = nullptr);
        static void write(FormattedSerializeStream &out, const Physics::CollisionShapeManager::InstanceHandle &handle, const char *name = nullptr);
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);
    };
}
}

REGISTER_TYPE(Engine::Physics::CollisionShapeManager)