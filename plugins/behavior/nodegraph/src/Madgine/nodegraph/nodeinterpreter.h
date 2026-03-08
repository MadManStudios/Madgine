#pragma once

#include "Meta/keyvalue/argumentlist.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/behaviorcollector.h"

#include "nodegraphloader.h"

namespace ax {
namespace NodeEditor {
    struct EditorContext;
}
}

namespace ed = ax::NodeEditor;

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT NodeDebugLocation {
            NodeDebugLocation(NodeInterpreterStateBase *interpreter)
                : mInterpreter(interpreter)
            {
            }            

            Debug::SenderLocation *mChild = nullptr;

            const NodeBase *mNode = nullptr;
            NodeInterpreterStateBase *mInterpreter;
            mutable std::unique_ptr<ed::EditorContext, void (*)(ed::EditorContext *)> mEditorContext = { nullptr, nullptr };
        };

        struct NodeInterpreterData {
            virtual ~NodeInterpreterData() = default;

            /* virtual bool readVar(ValueType &ref, std::string_view name) { return false; }
            virtual bool writeVar(std::string_view name, const ValueType &v) { return false; }
            virtual std::vector<std::string_view> variables() { return {}; }*/
        };

        struct MADGINE_NODEGRAPH_EXPORT NodeInterpreterStateBase : BehaviorReceiver {
            NodeInterpreterStateBase(const NodeGraph *graph, NodeGraphLoader::Handle handle);
            NodeInterpreterStateBase(const NodeInterpreterStateBase &) = delete;
            NodeInterpreterStateBase(NodeInterpreterStateBase &&) = default;
            virtual ~NodeInterpreterStateBase() = default;

            NodeInterpreterStateBase &operator=(const NodeInterpreterStateBase &) = delete;
            NodeInterpreterStateBase &operator=(NodeInterpreterStateBase &&) = default;

            void branch(BehaviorReceiver &receiver, uint32_t flowIn, NodeDebugLocation &location);
            void branch(BehaviorReceiver &receiver, Pin pin, NodeDebugLocation &location);

            KeyValueResult read(ValueType &retVal, Pin pin);

            KeyValueResult read(ValueType &retVal, uint32_t dataProvider);

            const NodeGraph *graph() const;

            const ArgumentList &arguments() const;

            std::unique_ptr<NodeInterpreterData> &data(uint32_t index);

            /* virtual bool readVar(ValueType &result, std::string_view name, bool recursive = true);
            virtual bool writeVar(std::string_view name, const ValueType &v);
            virtual std::vector<std::string_view> variables();*/

            void start();
            void stop();

        protected:
            NodeDebugLocation mDebugLocation;

            Debug::Continuation mContinuation;

        private:
            ArgumentList mArguments;

            const NodeGraph *mGraph;

            NodeGraphLoader::Handle mHandle;

            std::vector<std::unique_ptr<NodeInterpreterData>> mData;
        };

        template <typename Rec>
        using NodeInterpreterState = Execution::VirtualState<NodeInterpreterStateBase, Rec>;

        struct NodeInterpreterSender : Execution::base_sender {

            NodeInterpreterSender(const NodeGraph *graph)
                : mGraph(graph)
            {
            }

            NodeInterpreterSender(NodeGraphLoader::Handle handle)
                : mHandle(std::move(handle))
                , mGraph(mHandle)
            {
            }

            using result_type = KeyValueError;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<ArgumentList>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, NodeInterpreterSender &&sender, Rec &&rec)
            {
                return NodeInterpreterState<Rec> { std::forward<Rec>(rec), sender.mGraph, std::move(sender.mHandle) };
            }

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, NodeInterpreterSender &sender, Rec &&rec)
            {
                return NodeInterpreterState<Rec> { std::forward<Rec>(rec), sender.mGraph, sender.mHandle };
            }

            NodeGraphLoader::Handle mHandle;
            const NodeGraph *mGraph;
        };

    }
}
}