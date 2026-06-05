#pragma once

#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/resources/resourceloader.h"
#include "Madgine/resources/sender.h"

#include "nodegraph.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT NodeGraphLoader : Resources::ResourceLoader<NodeGraphLoader, NodeGraph, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<NodeGraphLoader, NodeGraph, std::list<Placeholder<0>>>;

            struct Handle : Base::Handle {
                using Base::Handle::Handle;
                Handle(Base::Handle handle)
                    : Base::Handle(std::move(handle))
                {
                }

                Resources::with_handle_t::sender<NodeInterpreterSender, Handle> interpret() const;
            };

            NodeGraphLoader();

            Threading::Task<bool> loadImpl(NodeGraph &graph, ResourceDataInfo &info);
            void unloadImpl(NodeGraph &graph);
        };

        struct NodeGraphBehaviorFactory : BehaviorFactory<NodeGraphBehaviorFactory> {
            std::vector<std::string_view> names() const override;
            UniqueOpaquePtr load(std::string_view name) const override;
            Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
            void release(UniqueOpaquePtr &ptr) const override;
            std::string_view name(const UniqueOpaquePtr &handle) const override;
            Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const override;
            ParameterTuple createParameters(const UniqueOpaquePtr &handle) const override;
            std::vector<Reflect::ExtendedType> parameterTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<Reflect::ExtendedType> resultTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<NamedDescriptor> namedInputs(const UniqueOpaquePtr &handle) const override;
            size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;
        };

    }
}
}