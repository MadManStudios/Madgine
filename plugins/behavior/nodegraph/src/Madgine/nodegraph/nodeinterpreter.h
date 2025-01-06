#pragma once

#include "Generic/genericresult.h"

#include "Madgine/behavior.h"
#include "Madgine/behaviorcollector.h"

#include "nodegraphloader.h"

#include "Meta/keyvalue/argumentlist.h"

namespace ax {
namespace NodeEditor {
    struct EditorContext;
}
}

namespace ed = ax::NodeEditor;

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT NodeDebugLocation : Debug::DebugLocation {
        NodeDebugLocation(NodeInterpreterStateBase *interpreter)
            : mInterpreter(interpreter)
        {
        }

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        bool wantsPause(Debug::ContinuationType type) const override;

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

        BehaviorError read(ValueType &retVal, Pin pin);
        void write(Pin pin, const ValueType &v);

        BehaviorError read(ValueType &retVal, uint32_t dataProvider);
        void write(uint32_t dataReceiver, const ValueType &v);

        const NodeGraph *graph() const;

        const ArgumentList &arguments() const;

        std::unique_ptr<NodeInterpreterData> &data(uint32_t index);

        /* virtual bool readVar(ValueType &result, std::string_view name, bool recursive = true);
        virtual bool writeVar(std::string_view name, const ValueType &v);
        virtual std::vector<std::string_view> variables();*/

        void start();

    protected:
        NodeDebugLocation mDebugLocation;

    private:
        ArgumentList mArguments;

        const NodeGraph *mGraph;

        NodeGraphLoader::Handle mHandle;

        std::vector<std::unique_ptr<NodeInterpreterData>> mData;
    };

    template <typename Rec>
    struct NodeInterpreterState : Execution::VirtualState<Rec, NodeInterpreterStateBase> {

        NodeInterpreterState(Rec &&rec, const NodeGraph *graph, NodeGraphLoader::Handle handle)
            : Execution::VirtualState<Rec, NodeInterpreterStateBase> { std::forward<Rec>(rec), graph, std::move(handle) }            
        {
        }

        void start()
        {
            this->mDebugLocation.stepInto(Execution::get_debug_location(this->mRec));
            NodeInterpreterStateBase::start();
        }

        virtual void set_done() override
        {
            this->mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            this->mRec.set_done();
        }
        virtual void set_error(BehaviorError r) override
        {
            this->mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            this->mRec.set_error(std::move(r));
        }
        virtual void set_value(ArgumentList result) override
        {
            this->mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            this->mRec.set_value(std::move(result));
        }
    };

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

        using result_type = BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<ArgumentList>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, NodeInterpreterSender &&sender, Rec &&rec)
        {
            return NodeInterpreterState<Rec> { std::forward<Rec>(rec), sender.mGraph, std::move(sender.mHandle) };
        }

        static constexpr size_t debug_operation_increment = 1;

        NodeGraphLoader::Handle mHandle;
        const NodeGraph *mGraph;
    };

}
}

REGISTER_TYPE(Engine::NodeGraph::NodeInterpreterStateBase);