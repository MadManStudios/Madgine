#include "../nodegraphlib.h"

#include "nodebase.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "nodegraph.h"

#include "nodes/util/sendernode.h"

METATABLE_BEGIN(Engine::NodeGraph::NodeBase)
METATABLE_END(Engine::NodeGraph::NodeBase)

SERIALIZETABLE_BEGIN(Engine::NodeGraph::NodeBase)
FIELD(mFlowOutPins)
FIELD(mDataInPins)
FIELD(mDataOutPins)
SERIALIZETABLE_END(Engine::NodeGraph::NodeBase)

namespace Engine {
namespace NodeGraph {

    Pin::operator bool() const
    {
        return bool(mIndex);
    }

    NodeBase::NodeBase(NodeGraph &graph)
        : mGraph(graph)
    {
    }

    NodeBase::NodeBase(const NodeBase &other, NodeGraph &graph)
        : mGraph(graph)
        , mFlowOutPins(other.mFlowOutPins)
        , mDataInPins(other.mDataInPins)
        , mDataOutPins(other.mDataOutPins)
        , mDataProviderPins(other.mDataProviderPins)
        , mDataReceiverPins(other.mDataReceiverPins)
        , mFlowInPins(other.mFlowInPins)
    {
    }

    uint32_t NodeBase::flowInId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 1 + index;
    }

    const std::vector<Pin> &NodeBase::flowInSources(uint32_t index, uint32_t group) const
    {
        return mFlowInPins[group][index].mSources;
    }

    uint32_t NodeBase::flowOutCount(uint32_t group) const
    {
        return mFlowOutPins[group].size();
    }

    Pin NodeBase::flowOutTarget(uint32_t index, uint32_t group) const
    {
        if (index == mFlowOutPins[group].size() && flowOutVariadic(group)) {
            return {};
        }
        return mFlowOutPins[group][index].mTarget;
    }

    uint32_t NodeBase::flowOutId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 1001 + index;
    }

    uint32_t NodeBase::dataInCount(uint32_t group) const
    {
        return mDataInPins[group].size();
    }

    ValueType NodeBase::dataInDefault(uint32_t index, uint32_t group) const
    {
        return {};
    }

    Pin NodeBase::dataInSource(uint32_t index, uint32_t group) const
    {
        if (index == mDataInPins[group].size() && dataInVariadic(group)) {
            return {};
        }
        return mDataInPins[group][index].mSource;
    }

    uint32_t NodeBase::dataInId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 2001 + index;
    }

    uint32_t NodeBase::dataOutCount(uint32_t group) const
    {
        return mDataOutPins[group].size();
    }

    Pin NodeBase::dataOutTarget(uint32_t index, uint32_t group) const
    {
        return mDataOutPins[group][index].mTarget;
    }

    uint32_t NodeBase::dataOutId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 3001 + index;
    }

    uint32_t NodeBase::dataReceiverCount(uint32_t group) const
    {
        return mDataReceiverPins[group].size();
    }

    const std::vector<Pin> &NodeBase::dataReceiverSources(uint32_t index, uint32_t group) const
    {
        return mDataReceiverPins[group][index].mSources;
    }

    uint32_t NodeBase::dataReceiverId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 4001 + index;
    }

    uint32_t NodeBase::dataProviderCount(uint32_t group) const
    {
        return mDataProviderPins[group].size();
    }

    const std::vector<Pin> &NodeBase::dataProviderTargets(uint32_t index, uint32_t group) const
    {
        return mDataProviderPins[group][index].mTargets;
    }

    uint32_t NodeBase::dataProviderId(uint32_t index, uint32_t group)
    {
        return 6000 * group + 5001 + index;
    }

    PinDesc NodeBase::pinFromId(uint32_t id)
    {
        uint32_t smallId = id % 6000;
        PinDesc desc;
        desc.mDir = ((smallId % 2000) < 1000 ? PinDir::In : PinDir::Out);
        desc.mType = (smallId < 4000 ? (smallId < 2000 ? PinType::Flow : PinType::Data) : PinType::DataInstance);
        desc.mPin = { id / 60000, id % 1000 - 1, (id / 6000) % 10 };
        return desc;
    }

    void NodeBase::onFlowOutUpdate(Pin source, EdgeEvent event)
    {
        switch (event) {
        case CONNECT:
            if (flowOutVariadic(source.mGroup) && source.mIndex == flowOutCount(source.mGroup))
                addVariadicPin(source.mGroup);
            break;
        case DISCONNECT:
            if (flowOutVariadic(source.mGroup) && source.mIndex >= flowOutBaseCount(source.mGroup))
                removeVariadicPin({ source.mNode, source.mIndex - dataInBaseCount(source.mGroup), source.mGroup });
            break;
        }
    }

    void NodeBase::onDataInUpdate(Pin target, EdgeEvent event)
    {
        switch (event) {
        case CONNECT:
            if (dataInVariadic(target.mGroup) && target.mIndex == dataInCount(target.mGroup))
                addVariadicPin(target.mGroup);
            break;
        case DISCONNECT:
            if (dataInVariadic(target.mGroup) && target.mIndex >= dataInBaseCount(target.mGroup))
                removeVariadicPin({ target.mNode, target.mIndex - dataInBaseCount(target.mGroup), target.mGroup });
            break;
        }
    }

    void NodeBase::onDataOutUpdate(Pin source, EdgeEvent event)
    {
    }

    void NodeBase::onFlowInUpdate(Pin target, EdgeEvent event)
    {
    }

    void NodeBase::onDataProviderUpdate(Pin source, EdgeEvent event)
    {
        switch (event) {
        case CONNECT:
            if (dataProviderVariadic(source.mGroup) && source.mIndex == dataProviderCount(source.mGroup))
                addVariadicPin(source.mGroup);
            break;
        case DISCONNECT:
            if (dataProviderVariadic(source.mGroup) && source.mIndex >= dataProviderBaseCount(source.mGroup))
                removeVariadicPin({ source.mNode, source.mIndex - dataProviderBaseCount(source.mGroup), source.mGroup });
            break;
        }
    }

    void NodeBase::onDataReceiverUpdate(Pin target, EdgeEvent event)
    {
    }

    void NodeBase::addVariadicPin(uint32_t group)
    {
        if (flowOutVariadic(group))
            mFlowOutPins[group].emplace_back();
        if (dataInVariadic(group))
            mDataInPins[group].emplace_back();
        if (dataOutVariadic(group))
            mDataOutPins[group].emplace_back();
        if (dataProviderVariadic(group))
            mDataProviderPins[group].emplace_back();
        if (dataReceiverVariadic(group))
            mDataReceiverPins[group].emplace_back();
    }

    void NodeBase::removeVariadicPin(Pin pin)
    {
        if (flowOutVariadic(pin.mGroup))
            removeFlowOutPin({ pin.mNode, pin.mIndex + flowOutBaseCount(pin.mGroup), pin.mGroup });
        if (dataInVariadic(pin.mGroup))
            removeDataInPin({ pin.mNode, pin.mIndex + dataInBaseCount(pin.mGroup), pin.mGroup });
        if (dataOutVariadic(pin.mGroup))
            removeFlowOutPin({ pin.mNode, pin.mIndex + dataOutBaseCount(pin.mGroup), pin.mGroup });
        if (dataProviderVariadic(pin.mGroup))
            removeDataProviderPin({ pin.mNode, pin.mIndex + dataProviderBaseCount(pin.mGroup), pin.mGroup });
        if (dataReceiverVariadic(pin.mGroup))
            removeDataReceiverPin({ pin.mNode, pin.mIndex + dataReceiverBaseCount(pin.mGroup), pin.mGroup });
    }

    size_t NodeBase::variadicPinCount(uint32_t group) const
    {
        if (flowOutVariadic(group))
            return flowOutCount(group) - flowOutBaseCount(group);
        if (dataInVariadic(group))
            return dataInCount(group) - dataInBaseCount(group);
        if (dataOutVariadic(group))
            return dataOutCount(group) - dataOutBaseCount(group);
        if (dataProviderVariadic(group))
            return dataProviderCount(group) - dataProviderBaseCount(group);
        if (dataReceiverVariadic(group))
            return dataReceiverCount(group) - dataReceiverBaseCount(group);
        return 0;
    }

    void NodeBase::removeFlowOutPin(Pin pin)
    {
        mGraph.onFlowOutRemove(pin);
        if (mFlowOutPins[pin.mGroup][pin.mIndex].mTarget)
            mGraph.disconnectFlow({ mGraph.nodeIndex(this), pin.mIndex, pin.mGroup }, { .mIgnoreSource=true });
        mFlowOutPins[pin.mGroup].erase(mFlowOutPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::removeFlowInPin(Pin pin)
    {
        mGraph.onFlowInRemove(pin);
        while (!mFlowInPins[pin.mGroup][pin.mIndex].mSources.empty()) {
            mGraph.disconnectFlow(mFlowInPins[pin.mGroup][pin.mIndex].mSources.front(), { .mIgnoreTarget = true });
        }
        mFlowInPins[pin.mGroup].erase(mFlowInPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::removeDataInPin(Pin pin)
    {
        mGraph.onDataInRemove(pin);
        if (mDataInPins[pin.mGroup][pin.mIndex].mSource)
            mGraph.disconnectDataIn(pin, { .mIgnoreTarget = true });
        mDataInPins[pin.mGroup].erase(mDataInPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::removeDataProviderPin(Pin pin)
    {
        mGraph.onDataProviderRemove(pin);
        while (!mDataProviderPins[pin.mGroup][pin.mIndex].mTargets.empty()) {
            mGraph.disconnectDataIn(mDataProviderPins[pin.mGroup][pin.mIndex].mTargets.front(), { .mIgnoreSource = true });
        }
        mDataProviderPins[pin.mGroup].erase(mDataProviderPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::removeDataOutPin(Pin pin)
    {
        mGraph.onDataOutRemove(pin);
        if (mDataOutPins[pin.mGroup][pin.mIndex].mTarget)
            mGraph.disconnectDataOut(pin, { .mIgnoreSource = true });
        mDataOutPins[pin.mGroup].erase(mDataOutPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::removeDataReceiverPin(Pin pin)
    {
        mGraph.onDataReceiverRemove(pin);
        while (!mDataReceiverPins[pin.mGroup][pin.mIndex].mSources.empty()) {
            mGraph.disconnectDataOut(mDataReceiverPins[pin.mGroup][pin.mIndex].mSources.front(), { .mIgnoreTarget = true });
        }
        mDataReceiverPins[pin.mGroup].erase(mDataReceiverPins[pin.mGroup].begin() + pin.mIndex);
    }

    void NodeBase::onFlowInRemove(Pin pin)
    {
        for (auto &group : mFlowInPins) {
            for (FlowInPinPrototype &inPin : group) {
                for (Pin &source : inPin.mSources) {
                    if (source.mNode == pin.mNode && source.mGroup == pin.mGroup) {
                        if (source && source.mIndex > pin.mIndex) {
                            --source.mIndex;
                        }
                    }
                }
            }
        }
    }

    void NodeBase::onFlowOutRemove(Pin pin)
    {
        for (auto &group : mFlowOutPins) {
            for (FlowOutPinPrototype &outPin : group) {
                Pin &target = outPin.mTarget;
                if (target.mNode == pin.mNode && target.mGroup == pin.mGroup) {
                    if (target && target.mIndex > pin.mIndex) {
                        --target.mIndex;
                    }
                }
            }
        }
    }

    void NodeBase::onDataOutRemove(Pin pin)
    {
        for (auto &group : mDataReceiverPins) {
            for (DataReceiverPinPrototype &receiver : group) {
                for (Pin &source : receiver.mSources) {
                    if (source.mNode == pin.mNode && source.mGroup == pin.mGroup) {
                        if (source && source.mIndex > pin.mIndex) {
                            --source.mIndex;
                        }
                    }
                }
            }
        }
    }

    void NodeBase::onDataReceiverRemove(Pin pin)
    {
        for (auto &group : mDataOutPins) {
            for (DataOutPinPrototype &outPin : group) {
                Pin &target = outPin.mTarget;
                if (target.mNode == pin.mNode && target.mGroup == pin.mGroup) {
                    if (target && target.mIndex > pin.mIndex) {
                        --target.mIndex;
                    }
                }
            }
        }
    }

    void NodeBase::onDataInRemove(Pin pin)
    {
        for (auto &group : mDataProviderPins) {
            for (DataProviderPinPrototype &provider : group) {
                for (Pin &target : provider.mTargets) {
                    if (target.mNode == pin.mNode && target.mGroup == pin.mGroup) {
                        if (target && target.mIndex > pin.mIndex) {
                            --target.mIndex;
                        }
                    }
                }
            }
        }
    }

    void NodeBase::onDataProviderRemove(Pin pin)
    {
        for (auto &group : mDataInPins) {
            for (DataInPinPrototype &inPin : group) {
                Pin &source = inPin.mSource;
                if (source.mNode == pin.mNode && source.mGroup == pin.mGroup) {
                    if (source && source.mIndex > pin.mIndex) {
                        --source.mIndex;
                    }
                }
            }
        }
    }

    void NodeBase::onNodeReindex(uint32_t oldIndex, uint32_t newIndex)
    {
        for (auto &group : mFlowOutPins) {
            for (FlowOutPinPrototype &pin : group) {
                assert(pin.mTarget.mNode != newIndex);
                if (pin.mTarget.mNode == oldIndex)
                    pin.mTarget.mNode = newIndex;
            }
        }
        for (auto &group : mDataInPins) {
            for (DataInPinPrototype &pin : group) {
                assert(pin.mSource.mNode != newIndex);
                if (pin.mSource.mNode == oldIndex)
                    pin.mSource.mNode = newIndex;
            }
        }
        for (auto &group : mDataOutPins) {
            for (DataOutPinPrototype &pin : group) {
                assert(pin.mTarget.mNode != newIndex);
                if (pin.mTarget.mNode == oldIndex)
                    pin.mTarget.mNode = newIndex;
            }
        }

        for (auto &group : mFlowInPins) {
            for (FlowInPinPrototype &pin : group) {
                for (Pin &source : pin.mSources) {
                    assert(source.mNode != newIndex);
                    if (source.mNode == oldIndex)
                        source.mNode = newIndex;
                }
            }
        }
        for (auto &group : mDataReceiverPins) {
            for (DataReceiverPinPrototype &pin : group) {
                for (Pin &source : pin.mSources) {
                    assert(source.mNode != newIndex);
                    if (source.mNode == oldIndex)
                        source.mNode = newIndex;
                }
            }
        }
        for (auto &group : mDataProviderPins) {
            for (DataProviderPinPrototype &pin : group) {
                for (Pin &target : pin.mTargets) {
                    assert(target.mNode != newIndex);
                    if (target.mNode == oldIndex)
                        target.mNode = newIndex;
                }
            }
        }
    }

    void NodeBase::setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const
    {
    }

    void NodeBase::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
    {
        throw 0;
    }

    BehaviorError NodeBase::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        throw 0;
    }

    void NodeBase::interpretWrite(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data, const ValueType &v, uint32_t receiverIndex, uint32_t group) const
    {
        throw 0;
    }

    void NodeBase::generate(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t flowIn, uint32_t group) const
    {
        throw 0;
    }

    CodeGen::Statement NodeBase::generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group) const
    {
        throw 0;
    }

    CodeGen::Statement NodeBase::generateWrite(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t receiverIndex, uint32_t group) const
    {
        throw 0;
    }

    void NodeBase::setup()
    {
        mFlowOutPins.resize(flowOutGroupCount());
        for (size_t i = 0; i < flowOutGroupCount(); ++i) {
            mFlowOutPins[i].resize(flowOutBaseCount(i));
        }
        mDataInPins.resize(dataInGroupCount());
        for (size_t i = 0; i < dataInGroupCount(); ++i) {
            mDataInPins[i].resize(dataInBaseCount(i));
        }
        mDataOutPins.resize(dataOutGroupCount());
        for (size_t i = 0; i < dataOutGroupCount(); ++i) {
            mDataOutPins[i].resize(dataOutBaseCount(i));
        }

        mFlowInPins.resize(flowInGroupCount());
        for (size_t i = 0; i < flowInGroupCount(); ++i) {
            mFlowInPins[i].resize(flowInCount(i));
        }
        mDataProviderPins.resize(dataProviderGroupCount());
        for (size_t i = 0; i < dataProviderGroupCount(); ++i) {
            mDataProviderPins[i].resize(dataProviderBaseCount(i));
        }
        mDataReceiverPins.resize(dataReceiverGroupCount());
        for (size_t i = 0; i < dataReceiverGroupCount(); ++i) {
            mDataReceiverPins[i].resize(dataReceiverBaseCount(i));
        }
    }

}
}
