#pragma once

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct NodeBase;
        struct NodeGraph;

        template <typename Node>
        struct NodeReceiver;

        struct NodeInterpreterData;
        struct NodeInterpreterStateBase;
        struct NodeInterpreterSender;

        struct Pin;

        struct FlowOutPinPrototype;
        struct DataInPinPrototype;
        struct DataOutPinPrototype;

        enum EdgeEvent {
            CONNECT,
            DISCONNECT
        };

        using NodeResults = Reflect::ArgumentList;

        struct NodeDebugLocation;

    }
}
}