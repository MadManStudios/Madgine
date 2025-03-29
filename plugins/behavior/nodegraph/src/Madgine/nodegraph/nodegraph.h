#pragma once

#include "Interfaces/filesystem/path.h"

#include "nodebase.h"

#include "Madgine/bindings.h"

namespace Engine {
namespace NodeGraph {

    struct Ignore {
        bool mIgnoreSource = false;
        bool mIgnoreTarget = false;
    };

    struct MADGINE_NODEGRAPH_EXPORT NodeGraph {

        SERIALIZABLEUNIT(NodeGraph)

        NodeGraph();
        NodeGraph(const NodeGraph &other);
        NodeGraph(NodeGraph &&);
        ~NodeGraph();

        NodeGraph &operator=(const NodeGraph &other);

        Serialize::StreamResult loadFromFile(const Filesystem::Path &path);
        void saveToFile(const Filesystem::Path &path);

        NodeBase *addNode(std::unique_ptr<NodeBase> node);
        NodeBase *addNode(std::string_view name);
        void removeNode(uint32_t index);

        const std::vector<std::unique_ptr<NodeBase>> &nodes() const;
        const NodeBase *node(IndexType<uint32_t, 0> index) const;
        NodeBase *node(IndexType<uint32_t, 0> index);
        uint32_t nodeIndex(const NodeBase *node) const;

        Pin flowOutTarget(Pin source);
        Pin dataInSource(Pin target);
        Pin dataOutTarget(Pin source);

        ExtendedValueTypeDesc dataReceiverType(Pin source, bool bidir = true);
        ExtendedValueTypeDesc dataProviderType(Pin target, bool bidir = true);
        ExtendedValueTypeDesc dataInType(Pin source, bool bidir = true);
        ExtendedValueTypeDesc dataOutType(Pin target, bool bidir = true);

        uint32_t flowInMask(Pin target, bool bidir = true);
        uint32_t flowOutMask(Pin source, bool bidir = true);
        uint32_t dataReceiverMask(Pin source, bool bidir = true);
        uint32_t dataProviderMask(Pin target, bool bidir = true);
        uint32_t dataInMask(Pin source, bool bidir = true);
        uint32_t dataOutMask(Pin target, bool bidir = true);

        std::string_view flowInName(Pin target);
        std::string_view flowOutName(Pin source);
        std::string_view dataReceiverName(Pin source);
        std::string_view dataProviderName(Pin target);
        std::string_view dataInName(Pin source);
        std::string_view dataOutName(Pin target);

        ExtendedValueTypeDesc resolveVariableType(std::string_view name) const;

        void connectFlow(Pin source, Pin target);
        void connectDataIn(Pin target, Pin source);
        void connectDataOut(Pin source, Pin target);

        void disconnectFlow(Pin source, Ignore ignore = {});
        void disconnectDataIn(Pin target, Ignore ignore = {});
        void disconnectDataOut(Pin source, Ignore ignore = {});

        void onFlowInRemove(Pin pin);
        void onFlowOutRemove(Pin pin);
        void onDataReceiverRemove(Pin pin);
        void onDataProviderRemove(Pin pin);
        void onDataInRemove(Pin pin);
        void onDataOutRemove(Pin pin);

        std::vector<FlowInPinPrototype> mFlowInPins;
        std::vector<FlowOutPinPrototype> mFlowOutPins;
        std::vector<DataReceiverPinPrototype> mDataReceiverPins;
        std::vector<DataProviderPinPrototype> mDataProviderPins;
        std::vector<DataInPinPrototype> mDataInPins;
        std::vector<DataOutPinPrototype> mDataOutPins;

        std::string mLayoutData;

        struct InputBinding {
            BindingDescriptor mDescriptor { "Unnamed", ExtendedValueTypeIndex { ExtendedValueTypeEnum::GenericType } };
            std::vector<Pin> mTargets;
        };
        std::vector<InputBinding> mInputBindings;

        NodeInterpreterSender interpret() const;

    protected:
        std::unique_ptr<NodeBase> createNode(std::string_view name);
        Serialize::StreamResult readNode(Serialize::FormattedSerializeStream &in, std::unique_ptr<NodeBase> &node);
        const char *writeNode(Serialize::FormattedSerializeStream &out, const std::unique_ptr<NodeBase> &node) const;

    private:
        std::vector<std::unique_ptr<NodeBase>> mNodes;
    };

}
}
