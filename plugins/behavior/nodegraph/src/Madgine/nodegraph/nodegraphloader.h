#pragma once

#include "Madgine/resources/resourceloader.h"

#include "nodegraph.h"

#include "Madgine/behaviorcollector.h"

#include "Madgine/resources/sender.h"

namespace Engine {
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

        bool loadImpl(NodeGraph &graph, ResourceDataInfo &info);
        void unloadImpl(NodeGraph &graph);

    };

    struct NodeGraphBehaviorFactory : BehaviorFactory<NodeGraphBehaviorFactory> {
        std::vector<std::string_view> names() const override;
        UniqueOpaquePtr load(std::string_view name) const override;
        Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
        void release(UniqueOpaquePtr &ptr) const override;
        std::string_view name(const UniqueOpaquePtr &handle) const override;
        Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const override;
        Threading::TaskFuture<ParameterTuple> createParameters(const UniqueOpaquePtr &handle) const override;
        ParameterTuple createDummyParameters(const UniqueOpaquePtr &handle) const override;
        std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const override;
        std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const override;
        std::vector<BindingDescriptor> bindings(const UniqueOpaquePtr &handle) const override;
        size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;
    };

}
}


DECLARE_BEHAVIOR_FACTORY(Engine::NodeGraph::NodeGraphBehaviorFactory)


REGISTER_TYPE(Engine::NodeGraph::NodeGraphLoader)