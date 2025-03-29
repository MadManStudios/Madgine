#pragma once

#include "../nodebase.h"
#include "../nodecollector.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT FunctionNode : Serialize::VirtualData<FunctionNode, VirtualScope<FunctionNode, NodeBase>> {

        FunctionNode(NodeGraph &graph, const FunctionTable *function);
        FunctionNode(const FunctionNode &other, NodeGraph &graph);

        std::string_view name() const override;
        std::string_view className() const override;
        std::unique_ptr<NodeBase> clone(NodeGraph &graph) const override;

        uint32_t flowInCount(uint32_t group) const override;

        uint32_t flowOutBaseCount(uint32_t group) const override;

        virtual uint32_t dataInBaseCount(uint32_t group = 0) const override;
        virtual std::string_view dataInName(uint32_t index, uint32_t group) const override;
        virtual ExtendedValueTypeDesc dataInType(uint32_t index, uint32_t group, bool bidir = true) const override;

        virtual uint32_t dataProviderBaseCount(uint32_t group = 0) const override;
        virtual ExtendedValueTypeDesc dataProviderType(uint32_t index, uint32_t group, bool bidir = true) const override;

        virtual BehaviorError interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override;

        virtual CodeGen::Statement generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group = 0) const override;

        const FunctionTable *getFunction() const;

        std::string_view getFunctionName() const;

    private:
        const FunctionTable *mFunction = nullptr;
    };

}
}

REGISTER_TYPE(Engine::NodeGraph::FunctionNode)
