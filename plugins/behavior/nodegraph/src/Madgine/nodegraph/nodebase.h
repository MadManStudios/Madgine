#pragma once

#include "pins.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Meta/keyvalue/virtualscope.h"

#include "nodeexecutionmask.h"

#include "Madgine/codegen/statement.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT NodeBase : Serialize::VirtualSerializableDataBase<VirtualScopeBase<>> {

        SERIALIZABLEUNIT(NodeBase)

        NodeBase(NodeGraph &graph);
        NodeBase(const NodeBase &other, NodeGraph &graph);
        virtual ~NodeBase() = default;

        virtual std::string_view name() const = 0;
        virtual std::string_view className() const = 0;

        virtual uint32_t flowInGroupCount() const { return 1; }
        virtual uint32_t flowInCount(uint32_t group = 0) const { return 0; }
        virtual std::string_view flowInName(uint32_t index, uint32_t group = 0) const { return "call"; }
        virtual uint32_t flowInMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        const std::vector<Pin> &flowInSources(uint32_t index, uint32_t group = 0) const;
        static uint32_t flowInId(uint32_t index, uint32_t group = 0);

        virtual uint32_t flowOutGroupCount() const { return 1; }
        uint32_t flowOutCount(uint32_t group = 0) const;
        virtual std::string_view flowOutName(uint32_t index, uint32_t group = 0) const { return "return"; }
        virtual uint32_t flowOutMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        virtual bool flowOutVariadic(uint32_t group = 0) const { return false; }
        Pin flowOutTarget(uint32_t index, uint32_t group = 0) const;
        static uint32_t flowOutId(uint32_t index, uint32_t group = 0);

        virtual uint32_t dataInGroupCount() const { return 1; }
        uint32_t dataInCount(uint32_t group = 0) const;
        virtual std::string_view dataInName(uint32_t index, uint32_t group = 0) const { return "read"; }
        virtual uint32_t dataInMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        virtual ExtendedValueTypeDesc dataInType(uint32_t index, uint32_t group = 0, bool bidir = true) const { throw 0; }
        virtual bool dataInVariadic(uint32_t group = 0) const { return false; }
        virtual ValueType dataInDefault(uint32_t index, uint32_t group = 0) const;
        Pin dataInSource(uint32_t index, uint32_t group = 0) const;
        static uint32_t dataInId(uint32_t index, uint32_t group = 0);

        virtual uint32_t dataOutGroupCount() const { return 1; }
        uint32_t dataOutCount(uint32_t group = 0) const;
        virtual std::string_view dataOutName(uint32_t index, uint32_t group = 0) const { return "write"; }
        virtual uint32_t dataOutMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        virtual ExtendedValueTypeDesc dataOutType(uint32_t index, uint32_t group = 0, bool bidir = true) const { throw 0; }
        virtual bool dataOutVariadic(uint32_t group = 0) const { return false; }
        Pin dataOutTarget(uint32_t index, uint32_t group = 0) const;
        static uint32_t dataOutId(uint32_t index, uint32_t group = 0);

        virtual uint32_t dataReceiverGroupCount() const { return 1; }
        uint32_t dataReceiverCount(uint32_t group = 0) const;
        virtual std::string_view dataReceiverName(uint32_t index, uint32_t group = 0) const { return "in"; }
        virtual uint32_t dataReceiverMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        virtual ExtendedValueTypeDesc dataReceiverType(uint32_t index, uint32_t group = 0, bool bidir = true) const { throw 0; }
        virtual bool dataReceiverVariadic(uint32_t group = 0) const { return false; }
        const std::vector<Pin> &dataReceiverSources(uint32_t index, uint32_t group = 0) const;
        static uint32_t dataReceiverId(uint32_t index, uint32_t group = 0);

        virtual uint32_t dataProviderGroupCount() const { return 1; }
        uint32_t dataProviderCount(uint32_t group = 0) const;
        virtual std::string_view dataProviderName(uint32_t index, uint32_t group = 0) const { return "out"; }
        virtual uint32_t dataProviderMask(uint32_t index, uint32_t group = 0, bool bidir = true) const { return NodeExecutionMask::CPU; }
        virtual ExtendedValueTypeDesc dataProviderType(uint32_t index, uint32_t group = 0, bool bidir = true) const { throw 0; }
        virtual bool dataProviderVariadic(uint32_t group = 0) const { return false; }
        const std::vector<Pin> &dataProviderTargets(uint32_t index, uint32_t group = 0) const;
        static uint32_t dataProviderId(uint32_t index, uint32_t group = 0);

        static PinDesc pinFromId(uint32_t id);

        virtual void onFlowOutUpdate(Pin source, EdgeEvent event);
        virtual void onDataInUpdate(Pin target, EdgeEvent event);
        virtual void onDataOutUpdate(Pin source, EdgeEvent event);

        virtual void onFlowInUpdate(Pin target, EdgeEvent event);
        virtual void onDataProviderUpdate(Pin source, EdgeEvent event);
        virtual void onDataReceiverUpdate(Pin target, EdgeEvent event);

        void addVariadicPin(uint32_t group);
        void removeVariadicPin(Pin pin);
        size_t variadicPinCount(uint32_t group) const;

        void removeFlowInPin(Pin pin);
        void removeFlowOutPin(Pin pin);
        void removeDataInPin(Pin pin);
        void removeDataOutPin(Pin pin);
        void removeDataReceiverPin(Pin pin);
        void removeDataProviderPin(Pin pin);

        void onFlowInRemove(Pin pin);
        void onFlowOutRemove(Pin pin);
        void onDataInRemove(Pin pin);
        void onDataOutRemove(Pin pin);
        void onDataReceiverRemove(Pin pin);
        void onDataProviderRemove(Pin pin);
        void onNodeReindex(uint32_t oldIndex, uint32_t newIndex);

        virtual void setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const;
        virtual void interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group = 0) const;
        virtual BehaviorError interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group = 0) const;
        virtual void interpretWrite(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data, const ValueType &v, uint32_t receiverIndex, uint32_t group = 0) const;

        virtual void generate(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t flowIn, uint32_t group = 0) const;
        virtual CodeGen::Statement generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group = 0) const;
        virtual CodeGen::Statement generateWrite(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t receiverIndex, uint32_t group = 0) const;

        virtual std::unique_ptr<NodeBase> clone(NodeGraph &graph) const = 0;

    protected:
        virtual uint32_t flowOutBaseCount(uint32_t group = 0) const { return 0; }
        virtual uint32_t dataInBaseCount(uint32_t group = 0) const { return 0; }
        virtual uint32_t dataOutBaseCount(uint32_t group = 0) const { return 0; }
        virtual uint32_t dataReceiverBaseCount(uint32_t group = 0) const { return 0; }
        virtual uint32_t dataProviderBaseCount(uint32_t group = 0) const { return 0; }

        void setup();

        NodeGraph &mGraph;

        friend struct NodeGraph;

        std::vector<std::vector<FlowOutPinPrototype>> mFlowOutPins;
        std::vector<std::vector<DataInPinPrototype>> mDataInPins;
        std::vector<std::vector<DataOutPinPrototype>> mDataOutPins;

        std::vector<std::vector<DataProviderPinPrototype>> mDataProviderPins;
        std::vector<std::vector<DataReceiverPinPrototype>> mDataReceiverPins;
        std::vector<std::vector<FlowInPinPrototype>> mFlowInPins;
    };

}
}

REGISTER_TYPE(Engine::NodeGraph::NodeBase)
