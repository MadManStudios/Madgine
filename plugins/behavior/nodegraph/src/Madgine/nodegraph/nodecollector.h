#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::NodeGraph, Node, NodeBase, Engine::UniqueComponent::Constructor<NodeGraph &>);

namespace Engine {
namespace NodeGraph {

    template <typename T, typename Base = NodeBase>
    struct Node : Serialize::VirtualData<T, VirtualScope<T, NodeComponent<T, Base>>> {
        using Serialize::VirtualData<T, VirtualScope<T, NodeComponent<T, Base>>>::VirtualData;

        virtual std::string_view className() const override final
        {
            return T::componentName();
        }
        virtual std::string_view name() const override
        {
            return className();
        }
        virtual std::unique_ptr<NodeBase> clone(NodeGraph& graph) const override {
            return std::make_unique<T>(*static_cast<const T *>(this), graph);
        }
    };

#define NODE(Name, FullType)              \
    NAMED_UNIQUECOMPONENT(Name, FullType)
}
}