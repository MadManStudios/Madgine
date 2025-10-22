#pragma once

namespace Engine {
namespace NodeGraph {

    struct NodeBase;
    struct NodeGraph;

    template <typename Node>
    struct NodeReceiver;

    struct NodeInterpreterData;
    struct NodeInterpreterStateBase;
    struct NodeInterpreterSender;

    struct CodeGenerator;
    struct CodeGeneratorData;

    struct Pin;

    struct FlowOutPinPrototype;
    struct DataInPinPrototype;
    struct DataOutPinPrototype;

    enum EdgeEvent {
        CONNECT,
        DISCONNECT
    };

    using NodeResults = ArgumentList;

    struct NodeDebugLocation;

}
}
