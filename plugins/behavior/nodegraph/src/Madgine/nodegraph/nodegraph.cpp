#include "../nodegraphlib.h"
#include "Madgine/serialize/filesystem/filesystemlib.h"

#include "nodegraph.h"

#include "Generic/projections.h"

#include "Platform/filesystem/fsapi.h"

#include "Meta/serialize/formats.h"
#include "Meta/serialize/streams/serializestream.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "nodebase.h"
#include "nodecollector.h"
#include "nodeinterpreter.h"
#include "nodes/accessornode.h"
#include "nodes/behaviornode.h"

METATABLE_BEGIN(Engine::Behavior::NodeGraph::NodeGraph)
METATABLE_END(Engine::Behavior::NodeGraph::NodeGraph)

SERIALIZETABLE_BEGIN(Engine::Behavior::NodeGraph::NodeGraph)
    FIELD(mNodes, Serialize::ParentCreator<&Engine::Behavior::NodeGraph::NodeGraph::readNode, &Engine::Behavior::NodeGraph::NodeGraph::writeNode>)
    FIELD(mFlowOutPins)
    FIELD(mDataInPins)
    FIELD(mLayoutData)
    FIELD(mNamedInputs)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::NodeGraph)

METATABLE_BEGIN(Engine::Behavior::NodeGraph::NodeGraph::NamedInput)
    //MEMBER(mDescriptor)
METATABLE_END(Engine::Behavior::NodeGraph::NodeGraph::NamedInput)

SERIALIZETABLE_BEGIN(Engine::Behavior::NodeGraph::NodeGraph::NamedInput)
    //FIELD(mDescriptor)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::NodeGraph::NamedInput)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {
        NodeGraph::NodeGraph()
        {
        }

        NodeGraph::NodeGraph(const NodeGraph &other)
            : mFlowInPins(other.mFlowInPins)
            , mFlowOutPins(other.mFlowOutPins)
            , mDataInPins(other.mDataInPins)
            , mDataOutPins(other.mDataOutPins)
            , mLayoutData(other.mLayoutData)
            , mNamedInputs(other.mNamedInputs)
        {
            mNodes.reserve(other.mNodes.size());
            std::ranges::transform(other.mNodes, std::back_inserter(mNodes), [&](const std::unique_ptr<NodeBase> &node) { return node->clone(*this); });
        }

        NodeGraph::NodeGraph(NodeGraph &&other) = default;
        NodeGraph::~NodeGraph()
        {
        }

        NodeGraph &NodeGraph::operator=(const NodeGraph &other)
        {
            mFlowInPins = other.mFlowInPins;
            mFlowOutPins = other.mFlowOutPins;
            mDataInPins = other.mDataInPins;
            mDataOutPins = other.mDataOutPins;
            mNamedInputs = other.mNamedInputs;

            mLayoutData = other.mLayoutData;

            mNodes.clear();
            mNodes.reserve(other.mNodes.size());
            std::ranges::transform(other.mNodes, std::back_inserter(mNodes), [&](const std::unique_ptr<NodeBase> &node) { return node->clone(*this); });

            return *this;
        }

        Threading::Task<Serialize::StreamResult> NodeGraph::loadFromFile(const Platform::Filesystem::Path &path)
        {
            if (Platform::Filesystem::exists(path)) {
                Serialize::FileManager mgr("Graph-Serializer");
                Serialize::FormattedSerializeStream in = mgr.openRead(path, Serialize::Formats::xml);

                Serialize::StreamResult result = Serialize::readState(in, *this, "Graph");
                if (result.mState != ::Engine::Serialize::StreamState::OK)
                    co_return std::move(result);

                for (NodeBase *node : mNodes | std::views::transform(projectionUniquePtrToPtr)) {
                    size_t maxGroupCount = std::max({ node->flowOutGroupCount(),
                        node->dataInGroupCount(),
                        node->dataOutGroupCount() });
                    for (size_t group = 0; group < maxGroupCount; ++group) {
                        IndexType<size_t> variadicCount;
                        if (group < node->flowOutGroupCount() && node->flowOutVariadic(group))
                            variadicCount = node->flowOutCount(group) - node->flowOutBaseCount(group);
                        if (group < node->dataInGroupCount() && node->dataInVariadic(group)) {
                            size_t count = node->dataInCount(group) - node->dataInBaseCount(group);
                            assert(!variadicCount || variadicCount == count);
                            variadicCount = count;
                        }
                        if (variadicCount) {
                            if (node->dataOutVariadic(group))
                                node->mDataOutPins[group].resize(node->dataOutBaseCount(group) + variadicCount);
                        }
                    }
                }

                uint32_t i = 0;
                for (FlowOutPinPrototype &flowOut : mFlowOutPins) {
                    if (flowOut.mTarget) {
                        this->node(flowOut.mTarget.mNode)->mFlowInPins[flowOut.mTarget.mGroup][flowOut.mTarget.mIndex].mSources.push_back({ 0, i });
                    }
                    ++i;
                }

                i = 0;
                for (DataInPinPrototype &dataIn : mDataInPins) {
                    if (dataIn.mSource) {
                        this->node(dataIn.mSource.mNode)->mDataOutPins[dataIn.mSource.mGroup][dataIn.mSource.mIndex].mTargets.push_back({ 0, i });
                    }
                    ++i;
                }

                std::vector<std::optional<FlowInPinPrototype>> inFlows;
                std::vector<std::optional<DataOutPinPrototype>> outPins;
                for (NodeBase *node : mNodes | std::views::transform(projectionUniquePtrToPtr)) {
                    for (uint32_t group = 0; group < node->dataInGroupCount(); ++group) {
                        for (uint32_t i = 0; i < node->dataInCount(group); ++i) {
                            Pin pin = node->dataInSource(i, group);
                            if (pin) {
                                if (!pin.mNode) {
                                    if (pin.mGroup == 0) {
                                        if (outPins.size() <= pin.mIndex)
                                            outPins.resize(pin.mIndex + 1);
                                        outPins[pin.mIndex] = DataOutPinPrototype { { { nodeIndex(node), i, group } } };
                                    } else {
                                        mNamedInputs[pin.mIndex].mTargets.push_back({ nodeIndex(node), i, group });
                                    }
                                } else {
                                    NodeBase *targetNode = this->node(pin.mNode);
                                    if (targetNode->dataOutCount(pin.mGroup) <= pin.mIndex) {
                                        node->mDataInPins[group][i].mSource = {};
                                    } else {
                                        targetNode->mDataOutPins[pin.mGroup][pin.mIndex].mTargets.push_back({ nodeIndex(node), i, group });
                                    }
                                }
                            }
                        }
                    }
                    for (uint32_t group = 0; group < node->flowOutGroupCount(); ++group) {
                        for (uint32_t i = 0; i < node->flowOutCount(group); ++i) {
                            Pin pin = node->flowOutTarget(i, group);
                            if (pin) {
                                if (!pin.mNode) {
                                    if (inFlows.size() <= pin.mIndex)
                                        inFlows.resize(pin.mIndex + 1);
                                    inFlows[pin.mIndex] = FlowInPinPrototype { { { nodeIndex(node), i, group } } };
                                } else {
                                    NodeBase *targetNode = this->node(pin.mNode);
                                    if (targetNode->mFlowInPins[pin.mGroup].size() <= pin.mIndex) {
                                        node->mFlowOutPins[group][i].mTarget = {};
                                    } else {
                                        targetNode->mFlowInPins[pin.mGroup][pin.mIndex].mSources.push_back({ nodeIndex(node), i, group });
                                    }
                                }
                            }
                        }
                    }
                }
                for ([[maybe_unused]] const std::optional<FlowInPinPrototype> &pin : inFlows) {
                    assert(pin);
                }
                for ([[maybe_unused]] const std::optional<DataOutPinPrototype> &pin : outPins) {
                    assert(pin);
                }
                mFlowInPins.clear();
                std::transform(std::make_move_iterator(inFlows.begin()), std::make_move_iterator(inFlows.end()), std::back_inserter(mFlowInPins),
                    [](std::optional<FlowInPinPrototype> &&pin) { return FlowInPinPrototype { std::move(*pin) }; });
                mDataOutPins.clear();
                std::transform(std::make_move_iterator(outPins.begin()), std::make_move_iterator(outPins.end()), std::back_inserter(mDataOutPins),
                    [](std::optional<DataOutPinPrototype> &&pin) { return DataOutPinPrototype { std::move(*pin) }; });
            }
            co_return {};
        }

        void NodeGraph::saveToFile(const Platform::Filesystem::Path &path)
        {
            Serialize::FileManager mgr("Graph-Serializer");
            Serialize::FormattedSerializeStream out = mgr.openWrite(path, Serialize::Formats::xml);
            Serialize::write(out, *this, "Graph");
        }

        NodeBase *NodeGraph::addNode(std::unique_ptr<NodeBase> node)
        {
            mNodes.push_back(std::move(node));
            return mNodes.back().get();
        }

        NodeBase *NodeGraph::addNode(std::string_view name)
        {
            return addNode(createNode(name));
        }

        void NodeGraph::removeNode(uint32_t index)
        {
            assert(index != 0);

            NodeBase *node = mNodes[index - 1].get();

            uint32_t pin;
            uint32_t size;

            for (uint32_t group = 0; group < node->mFlowInPins.size(); ++group) {
                pin = 0;
                size = node->mFlowInPins[group].size();
                while (pin < size) {
                    const FlowInPinPrototype &flowIn = node->mFlowInPins[group][pin];
                    std::vector<Pin> sources = flowIn.mSources;
                    for (const Pin &source : sources) {
                        disconnectFlow(source);
                    }
                    uint32_t newSize = node->mFlowInPins[group].size();
                    if (size == newSize)
                        ++pin;
                    else
                        size = newSize;
                }
            }

            for (uint32_t group = 0; group < node->mFlowOutPins.size(); ++group) {
                pin = 0;
                size = node->mFlowOutPins[group].size();
                while (pin < size) {
                    const FlowOutPinPrototype &flowOut = node->mFlowOutPins[group][pin];
                    if (flowOut.mTarget) {
                        disconnectFlow({ index, pin, group });
                        uint32_t newSize = node->mFlowOutPins[group].size();
                        if (size == newSize)
                            ++pin;
                        else
                            size = newSize;
                    } else {
                        ++pin;
                    }
                }
            }

            for (uint32_t group = 0; group < node->mDataOutPins.size(); ++group) {
                pin = 0;
                size = node->mDataOutPins[group].size();
                while (pin < size) {
                    const DataOutPinPrototype &provider = node->mDataOutPins[group][pin];
                    std::vector<Pin> targets = provider.mTargets;
                    for (const Pin &target : targets) {
                        disconnectData(target);
                    }
                    uint32_t newSize = node->mDataOutPins[group].size();
                    if (size == newSize)
                        ++pin;
                    else
                        size = newSize;
                }
            }

            for (uint32_t group = 0; group < node->mDataInPins.size(); ++group) {
                pin = 0;
                size = node->mDataInPins[group].size();
                while (pin < size) {
                    const DataInPinPrototype &dataIn = node->mDataInPins[group][pin];
                    if (dataIn.mSource) {
                        disconnectData({ index, pin, group });
                        uint32_t newSize = node->mDataInPins[group].size();
                        if (size == newSize)
                            ++pin;
                        else
                            size = newSize;
                    } else {
                        ++pin;
                    }
                }
            }

            uint32_t oldIndex = mNodes.size();
            std::swap(mNodes[index - 1], mNodes.back());
            mNodes.pop_back();

            for (const std::unique_ptr<NodeBase> &node : mNodes)
                node->onNodeReindex(oldIndex, index);

            for (FlowOutPinPrototype &pin : mFlowOutPins) {
                assert(pin.mTarget.mNode != index);
                if (pin.mTarget.mNode == oldIndex)
                    pin.mTarget.mNode = index;
            }
            for (DataInPinPrototype &pin : mDataInPins) {
                assert(pin.mSource.mNode != index);
                if (pin.mSource.mNode == oldIndex)
                    pin.mSource.mNode = index;
            }

            for (DataOutPinPrototype &pin : mDataOutPins) {
                for (Pin &target : pin.mTargets) {
                    assert(target.mNode != index);
                    if (target.mNode == oldIndex)
                        target.mNode = index;
                }
            }
        }

        const std::vector<std::unique_ptr<NodeBase>> &NodeGraph::nodes() const
        {
            return mNodes;
        }

        const NodeBase *NodeGraph::node(IndexType<uint32_t, 0> index) const
        {
            if (index)
                return mNodes[index - 1].get();
            else
                return nullptr;
        }

        NodeBase *NodeGraph::node(IndexType<uint32_t, 0> index)
        {
            if (index)
                return mNodes[index - 1].get();
            else
                return nullptr;
        }

        uint32_t NodeGraph::nodeIndex(const NodeBase *node) const
        {
            if (!node)
                return 0;
            return std::ranges::find(mNodes, node, projectionToRawPtr) - mNodes.begin() + 1;
        }

        Pin NodeGraph::flowOutTarget(Pin source)
        {
            if (source.mNode)
                return node(source.mNode)->flowOutTarget(source.mIndex, source.mGroup);
            if (source.mIndex == mFlowOutPins.size())
                return {};
            return mFlowOutPins[source.mIndex].mTarget;
        }

        uint32_t NodeGraph::flowOutMask(Pin source, bool bidir)
        {
            if (source.mNode)
                return node(source.mNode)->flowOutMask(source.mIndex, source.mGroup, bidir);
            if (!bidir || source.mIndex == mFlowOutPins.size())
                return NodeExecutionMask::ALL;
            Pin target = mFlowOutPins[source.mIndex].mTarget;
            return node(target.mNode)->flowInMask(target.mIndex, target.mGroup, false);
        }

        uint32_t NodeGraph::NodeGraph::flowInMask(Pin target, bool bidir)
        {
            if (target.mNode)
                return node(target.mNode)->flowInMask(target.mIndex, target.mGroup, bidir);
            if (!bidir || target.mIndex == mFlowInPins.size())
                return NodeExecutionMask::ALL;
            Pin source = mFlowInPins[target.mIndex].mSources.front();
            return node(source.mNode)->flowOutMask(source.mIndex, source.mGroup, false);
        }

        Pin NodeGraph::dataInSource(Pin target)
        {
            if (target.mNode)
                return node(target.mNode)->dataInSource(target.mIndex, target.mGroup);
            if (target.mIndex == mDataInPins.size())
                return {};
            return mDataInPins[target.mIndex].mSource;
        }

        Reflect::ExtendedType NodeGraph::dataOutType(Pin target, bool bidir)
        {
            if (target.mNode)
                return node(target.mNode)->dataOutType(target.mIndex, target.mGroup, bidir);
            if (!bidir || target.mIndex == mDataOutPins.size())
                return { Reflect::ExtendedTypeEnum::GenericType };
            Pin source = mDataOutPins[target.mIndex].mTargets.front();
            return node(source.mNode)->dataInType(source.mIndex, source.mGroup, false);
        }

        Reflect::ExtendedType NodeGraph::dataInType(Pin source, bool bidir)
        {
            if (source.mNode)
                return node(source.mNode)->dataInType(source.mIndex, source.mGroup, bidir);
            if (!bidir || source.mIndex == mDataInPins.size())
                return { Reflect::ExtendedTypeEnum::GenericType };
            Pin target = mDataInPins[source.mIndex].mSource;
            return node(target.mNode)->dataOutType(target.mIndex, target.mGroup, false);
        }

        uint32_t NodeGraph::dataOutMask(Pin target, bool bidir)
        {
            if (target.mNode)
                return node(target.mNode)->dataOutMask(target.mIndex, target.mGroup, bidir);
            if (!bidir || target.mIndex == mDataOutPins.size())
                return NodeExecutionMask::ALL;
            Pin source = mDataOutPins[target.mIndex].mTargets.front();
            return node(source.mNode)->dataInMask(source.mIndex, source.mGroup, false);
        }

        uint32_t NodeGraph::dataInMask(Pin source, bool bidir)
        {
            if (source.mNode)
                return node(source.mNode)->dataInMask(source.mIndex, source.mGroup, bidir);
            if (!bidir || source.mIndex == mDataInPins.size())
                return NodeExecutionMask::ALL;
            Pin target = mDataInPins[source.mIndex].mSource;
            return node(target.mNode)->dataOutMask(target.mIndex, target.mGroup, false);
        }

        std::string_view NodeGraph::flowOutName(Pin source)
        {
            return node(source.mNode)->flowOutName(source.mIndex, source.mGroup);
        }

        std::string_view NodeGraph::flowInName(Pin target)
        {
            return node(target.mNode)->flowInName(target.mIndex, target.mGroup);
        }

        std::string_view NodeGraph::dataOutName(Pin target)
        {
            return target.mNode ? node(target.mNode)->dataOutName(target.mIndex, target.mGroup) : "graphInput";
        }

        std::string_view NodeGraph::dataInName(Pin source)
        {
            return node(source.mNode)->dataInName(source.mIndex, source.mGroup);
        }

        void NodeGraph::connectFlow(Pin source, Pin target)
        {
            if (!target.mNode) {
                assert(mFlowInPins.size() >= target.mIndex);
                if (mFlowInPins.size() == target.mIndex)
                    mFlowInPins.emplace_back();
                mFlowInPins[target.mIndex].mSources.push_back(source);
            } else {
                node(target.mNode)->onFlowInUpdate(target, CONNECT);
                node(target.mNode)->mFlowInPins[target.mGroup][target.mIndex].mSources.push_back(source);
            }
            if (!source.mNode) {
                assert(mFlowOutPins.size() >= source.mIndex);
                if (mFlowOutPins.size() == source.mIndex)
                    mFlowOutPins.resize(source.mIndex + 1);
                mFlowOutPins[source.mIndex] = { target };
            } else {
                node(source.mNode)->onFlowOutUpdate(source, CONNECT);
                node(source.mNode)->mFlowOutPins[source.mGroup][source.mIndex].mTarget = target;
            }
        }

        void NodeGraph::connectData(Pin target, Pin source)
        {
            if (!source.mNode) {
                assert(source.mGroup < 2);
                if (source.mGroup == 0) {
                    assert(mDataOutPins.size() >= source.mIndex);
                    if (mDataOutPins.size() == source.mIndex)
                        mDataOutPins.emplace_back();
                    mDataOutPins[source.mIndex].mTargets.push_back(target);
                } else {
                    mNamedInputs[source.mIndex].mTargets.push_back(target);
                }
            } else {
                node(source.mNode)->onDataOutUpdate(source, CONNECT);
                node(source.mNode)->mDataOutPins[source.mGroup][source.mIndex].mTargets.push_back(target);
            }

            if (!target.mNode) {
                assert(mDataInPins.size() <= target.mIndex);
                if (mDataInPins.size() == target.mIndex)
                    mDataInPins.emplace_back(DataInPinPrototype { source });
            } else {
                node(target.mNode)->onDataInUpdate(target, CONNECT);
                node(target.mNode)->mDataInPins[target.mGroup][target.mIndex].mSource = source;
            }
        }

        void NodeGraph::disconnectFlow(Pin source, Ignore ignore)
        {
            Pin target;
            if (!source.mNode) {
                target = mFlowOutPins[source.mIndex].mTarget;
                mFlowOutPins.erase(mFlowOutPins.begin() + source.mIndex);
            } else {
                target = node(source.mNode)->mFlowOutPins[source.mGroup][source.mIndex].mTarget;
                node(source.mNode)->mFlowOutPins[source.mGroup][source.mIndex] = {};
            }

            if (!target.mNode) {
                mFlowInPins.erase(mFlowInPins.begin() + target.mIndex);
            } else {
                /*auto result = */ std::erase(node(target.mNode)->mFlowInPins[target.mGroup][target.mIndex].mSources, source);
                /*assert(result == 1);*/
                if (!ignore.mIgnoreTarget)
                    node(target.mNode)->onFlowInUpdate(target, DISCONNECT);
            }

            if (source.mNode && !ignore.mIgnoreSource)
                node(source.mNode)->onFlowOutUpdate(source, DISCONNECT);
        }

        void NodeGraph::disconnectData(Pin target, Ignore ignore)
        {
            Pin source;
            if (!target.mNode) {
                source = mDataInPins[target.mIndex].mSource;
                mDataInPins.erase(mDataInPins.begin() + target.mIndex);

            } else {
                source = node(target.mNode)->mDataInPins[target.mGroup][target.mIndex].mSource;
                node(target.mNode)->mDataInPins[target.mGroup][target.mIndex] = {};
            }

            if (!source.mNode) {
                std::erase(mDataOutPins[source.mIndex].mTargets, target);
                if (mDataOutPins[source.mIndex].mTargets.empty()) {
                    onDataOutRemove(source);
                    mDataOutPins.erase(mDataOutPins.begin() + source.mIndex);
                }
            } else {
                [[maybe_unused]] auto result = std::erase(node(source.mNode)->mDataOutPins[source.mGroup][source.mIndex].mTargets, target);
                assert(result == 1);
                if (!ignore.mIgnoreSource)
                    node(source.mNode)->onDataOutUpdate(source, DISCONNECT);
            }

            if (target.mNode && !ignore.mIgnoreTarget)
                node(target.mNode)->onDataInUpdate(target, DISCONNECT);
        }

        void NodeGraph::onFlowOutRemove(Pin pin)
        {
            for (const std::unique_ptr<NodeBase> &ptr : mNodes) {
                if (nodeIndex(ptr.get()) != pin.mNode)
                    ptr->onFlowOutRemove(pin);
            }

            for (FlowInPinPrototype &inPin : mFlowInPins) {
                for (Pin &source : inPin.mSources) {
                    if (source.mNode == pin.mNode && source.mGroup == pin.mGroup) {
                        if (source && source.mIndex > pin.mIndex) {
                            --source.mIndex;
                        }
                    }
                }
            }
        }

        void NodeGraph::onFlowInRemove(Pin pin)
        {
            for (const std::unique_ptr<NodeBase> &ptr : mNodes) {
                if (nodeIndex(ptr.get()) != pin.mNode)
                    ptr->onFlowInRemove(pin);
            }

            for (FlowOutPinPrototype &outPin : mFlowOutPins) {
                Pin &target = outPin.mTarget;
                if (target.mNode == pin.mNode && target.mGroup == pin.mGroup) {
                    if (target && target.mIndex > pin.mIndex) {
                        --target.mIndex;
                    }
                }
            }
        }

        void NodeGraph::onDataInRemove(Pin pin)
        {
            for (const std::unique_ptr<NodeBase> &ptr : mNodes) {
                if (nodeIndex(ptr.get()) != pin.mNode)
                    ptr->onDataInRemove(pin);
            }

            for (DataOutPinPrototype &provider : mDataOutPins) {
                for (Pin &target : provider.mTargets) {
                    if (target.mNode == pin.mNode && target.mGroup == pin.mGroup) {
                        if (target && target.mIndex > pin.mIndex) {
                            --target.mIndex;
                        }
                    }
                }
            }
        }

        void NodeGraph::onDataOutRemove(Pin pin)
        {
            for (const std::unique_ptr<NodeBase> &ptr : mNodes) {
                if (nodeIndex(ptr.get()) != pin.mNode)
                    ptr->onDataOutRemove(pin);
            }

            for (DataInPinPrototype &inPin : mDataInPins) {
                Pin &source = inPin.mSource;
                if (source.mNode == pin.mNode && source.mGroup == pin.mGroup) {
                    if (source && source.mIndex > pin.mIndex) {
                        --source.mIndex;
                    }
                }
            }
        }

        NodeInterpreterSender NodeGraph::interpret() const
        {
            return { this };
        }

        std::unique_ptr<NodeBase> NodeGraph::createNode(std::string_view name)
        {
            return construct(NodeRegistry::get(NodeRegistry::sComponentsByName().at(name)), *this);
        }

        Serialize::StreamResult NodeGraph::readNode(Serialize::FormattedSerializeStream &in, std::unique_ptr<NodeBase> &node)
        {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Node", 1));

            std::string name;
            STREAM_PROPAGATE_ERROR(read(in, name, "type"));

            bool isNativeNode = NodeRegistry::sComponentsByName().contains(name);
            BehaviorHandle behavior;
            bool isBehaviorNode = behavior.fromString(name);

            bool isAccessor = StringUtil::startsWith(name, "Accessor/");

            if (!isNativeNode && !isBehaviorNode && !isAccessor)
                return STREAM_INTEGRITY_ERROR(in) << "No Node \"" << name << "\" available.\n"
                                                  << "Make sure to check the loaded plugins.";

            if (isNativeNode + isBehaviorNode + isAccessor > 1)
                return STREAM_INTEGRITY_ERROR(in) << "Multiple Nodes found with same name: " << name;

            if (isNativeNode) {
                node = createNode(name);
            } else if (isAccessor) {
                node = std::make_unique<AccessorNode>(*this, name);
            } else {                
                node = std::make_unique<BehaviorNode>(*this, behavior);
            }

            return {};
        }

        const char *NodeGraph::writeNode(Serialize::FormattedSerializeStream &out, const std::unique_ptr<NodeBase> &node) const
        {
            out.beginExtendedWrite("Node", 1);

            write(out, node->className(), "type");

            return "Node";
        }

    }
}
}