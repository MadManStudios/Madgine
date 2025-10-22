#pragma once

#include "../nodebase.h"
#include "../nodecollector.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT AccessorNode : Serialize::VirtualData<AccessorNode, VirtualScope<AccessorNode, NodeBase>> {

        AccessorNode(NodeGraph &graph, std::string_view fullClassName);
        AccessorNode(const AccessorNode &other, NodeGraph &graph);

        std::string_view name() const override;
        std::string_view className() const override;
        std::unique_ptr<NodeBase> clone(NodeGraph &graph) const override;

        uint32_t flowInCount(uint32_t group) const override;

        uint32_t flowOutBaseCount(uint32_t group) const override;

        uint32_t dataInBaseCount(uint32_t group = 0) const override;
        std::string_view dataInName(uint32_t index, uint32_t group) const override;
        ExtendedValueTypeDesc dataInType(uint32_t index, uint32_t group, bool bidir = true) const override;

        uint32_t dataProviderBaseCount(uint32_t group = 0) const override;
        ExtendedValueTypeDesc dataProviderType(uint32_t index, uint32_t group, bool bidir = true) const override;

        BehaviorError interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override;

        CodeGen::Statement generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group = 0) const override;

    protected:
        const MetaTable *type() const;
        const Accessor *accessor() const;

    private:
        std::string mFullClassName;
    };

}
}
