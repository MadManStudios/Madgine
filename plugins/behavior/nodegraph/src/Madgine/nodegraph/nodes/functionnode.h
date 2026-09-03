#pragma once

#include "../nodebase.h"
#include "../nodecollector.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT FunctionNode : Serialize::VirtualData<FunctionNode, Reflect::VirtualScope<FunctionNode, NodeBase>> {

            FunctionNode(NodeGraph &graph, std::string_view fullClassName);
            FunctionNode(const FunctionNode &other, NodeGraph &graph);

            std::string_view name() const override;
            std::string_view className() const override;
            std::unique_ptr<NodeBase> clone(NodeGraph &graph) const override;

            uint32_t flowInCount(uint32_t group) const override;

            uint32_t flowOutBaseCount(uint32_t group) const override;

            virtual uint32_t dataInBaseCount(uint32_t group = 0) const override;
            virtual std::string_view dataInName(uint32_t index, uint32_t group) const override;
            virtual Reflect::ExtendedType dataInType(uint32_t index, uint32_t group, bool bidir = true) const override;

            virtual uint32_t dataOutBaseCount(uint32_t group = 0) const override;
            virtual Reflect::ExtendedType dataOutType(uint32_t index, uint32_t group, bool bidir = true) const override;

            void setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const override;
            void interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const override;
            virtual Reflect::Result interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override;

        protected:
            const Reflect::FunctionTable *function() const;            

        private:
            std::string mFullClassName;
        };

    }
}
}
