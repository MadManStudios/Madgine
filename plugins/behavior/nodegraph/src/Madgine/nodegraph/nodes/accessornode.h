#pragma once

#include "../nodebase.h"
#include "../nodecollector.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT AccessorNode : Serialize::VirtualData<AccessorNode, Reflect::VirtualScope<AccessorNode, NodeBase>> {

            AccessorNode(NodeGraph &graph, std::string_view fullClassName);
            AccessorNode(const AccessorNode &other, NodeGraph &graph);

            std::string_view name() const override;
            std::string_view className() const override;
            std::unique_ptr<NodeBase> clone(NodeGraph &graph) const override;

            uint32_t flowInCount(uint32_t group) const override;

            uint32_t flowOutBaseCount(uint32_t group) const override;

            uint32_t dataInBaseCount(uint32_t group = 0) const override;
            std::string_view dataInName(uint32_t index, uint32_t group) const override;
            Reflect::ExtendedType dataInType(uint32_t index, uint32_t group, bool bidir = true) const override;

            uint32_t dataOutBaseCount(uint32_t group = 0) const override;
            Reflect::ExtendedType dataOutType(uint32_t index, uint32_t group, bool bidir = true) const override;

            Reflect::Result interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override;

        protected:
            const Reflect::MetaTable *type() const;
            const Reflect::Accessor *accessor() const;

        private:
            std::string mFullClassName;
        };

    }
}
}