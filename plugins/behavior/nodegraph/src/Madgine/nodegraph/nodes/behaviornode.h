#pragma once

#include "../nodebase.h"
#include "../nodecollector.h"

#include "Madgine/behavior/parametertuple.h"

#include "Madgine/behavior/behaviorhandle.h"

#include "Madgine/behavior/named.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {
        // TODO rename to BehaviorNode
        struct MADGINE_NODEGRAPH_EXPORT BehaviorNode : Serialize::VirtualData<BehaviorNode, VirtualScope<BehaviorNode, NodeBase>> {

            BehaviorNode(NodeGraph &graph, BehaviorHandle behavior, Threading::TaskFuture<bool> &future);
            BehaviorNode(NodeGraph &graph, BehaviorHandle behavior);
            BehaviorNode(const BehaviorNode &other, NodeGraph &graph);

            std::string_view name() const override;
            std::string_view className() const override;
            std::unique_ptr<NodeBase> clone(NodeGraph &graph) const override;

            uint32_t flowInCount(uint32_t group) const override;

            uint32_t flowOutGroupCount() const override;
            uint32_t flowOutBaseCount(uint32_t group) const override;
            std::string_view flowOutName(uint32_t index, uint32_t group = 0) const override;

            uint32_t dataInGroupCount() const override;
            uint32_t dataInBaseCount(uint32_t group) const override;
            std::string_view dataInName(uint32_t index, uint32_t group) const override;
            ExtendedValueTypeDesc dataInType(uint32_t index, uint32_t group, bool bidir = true) const override;

            uint32_t dataOutBaseCount(uint32_t group) const override;
            ExtendedValueTypeDesc dataOutType(uint32_t index, uint32_t group = 0, bool bidir = true) const override;

            void setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const override;
            void interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const override;
            BehaviorError interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group = 0) const override;

        private:
            BehaviorHandle mBehavior;
            std::string mFullClassName;

        public:
            ParameterTuple mParameters;
            std::vector<NamedDescriptor> mNamedInputs;
            uint32_t mSubBehaviorCount = 0;
        };

    }
}
}