#pragma once

#include "Platform/filesystem/path.h"

#include "Madgine/behavior/nameddescriptor.h"

#include "nodebase.h"

namespace Engine {
namespace Behavior {
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

            Threading::Task<Serialize::StreamResult> loadFromFile(const Platform::Filesystem::Path &path);
            void saveToFile(const Platform::Filesystem::Path &path);

            NodeBase *addNode(std::unique_ptr<NodeBase> node);
            NodeBase *addNode(std::string_view name);
            void removeNode(uint32_t index);

            const std::vector<std::unique_ptr<NodeBase>> &nodes() const;
            const NodeBase *node(IndexType<uint32_t, 0> index) const;
            NodeBase *node(IndexType<uint32_t, 0> index);
            uint32_t nodeIndex(const NodeBase *node) const;

            Pin flowOutTarget(Pin source);
            Pin dataInSource(Pin target);

            Reflect::ExtendedType dataInType(Pin source, bool bidir = true);
            Reflect::ExtendedType dataOutType(Pin target, bool bidir = true);

            uint32_t flowInMask(Pin target, bool bidir = true);
            uint32_t flowOutMask(Pin source, bool bidir = true);
            uint32_t dataInMask(Pin source, bool bidir = true);
            uint32_t dataOutMask(Pin target, bool bidir = true);

            std::string_view flowInName(Pin target);
            std::string_view flowOutName(Pin source);
            std::string_view dataInName(Pin source);
            std::string_view dataOutName(Pin target);

            Reflect::ExtendedType resolveVariableType(std::string_view name) const;

            void connectFlow(Pin source, Pin target);
            void connectData(Pin target, Pin source);

            void disconnectFlow(Pin source, Ignore ignore = {});
            void disconnectData(Pin target, Ignore ignore = {});

            void onFlowInRemove(Pin pin);
            void onFlowOutRemove(Pin pin);
            void onDataInRemove(Pin pin);
            void onDataOutRemove(Pin pin);

            std::vector<FlowInPinPrototype> mFlowInPins;
            std::vector<FlowOutPinPrototype> mFlowOutPins;
            std::vector<DataInPinPrototype> mDataInPins;
            std::vector<DataOutPinPrototype> mDataOutPins;

            std::string mLayoutData;

            struct NamedInput {
                NamedDescriptor mDescriptor { "Unnamed", Reflect::ExtendedTypeIndex { Reflect::ExtendedTypeEnum::GenericType } };
                std::vector<Pin> mTargets;
            };
            std::vector<NamedInput> mNamedInputs;

            NodeInterpreterSender interpret() const;

        protected:
            std::unique_ptr<NodeBase> createNode(std::string_view name);
            Serialize::StreamResult readNode(Serialize::CallerHierarchyFormattedSerializeStream in, std::unique_ptr<NodeBase> &node);
            const char *writeNode(Serialize::CallerHierarchyFormattedSerializeStream out, const std::unique_ptr<NodeBase> &node) const;

        private:
            std::vector<std::unique_ptr<NodeBase>> mNodes;
        };

    }
}
}