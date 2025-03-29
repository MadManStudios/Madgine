#pragma once


#include "sendernode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

namespace Engine {
namespace NodeGraph {

    struct DefaultConfig {
        static constexpr bool constant = false;
        using dynamicVariableNames = Engine::type_pack<>;
    };

    struct ConstantConfig {
        static constexpr bool constant = true;
        using dynamicVariableNames = Engine::type_pack<>;
    };

}
}

#define SENDER_NODE_BEGIN(Name, Config, ...)                               \
    using Name##Node = Engine::NodeGraph::SenderNode<Config, __VA_ARGS__>; \
                                                                           \
    REGISTER_TYPE(Name##Node)                                              \
    NAMED_UNIQUECOMPONENT(Name, Name##Node)                                \
                                                                           \
    METATABLE_BEGIN_BASE(Name##Node, Engine::NodeGraph::NodeBase)          \
    SERIALIZETABLE_INHERIT_BEGIN(Name##Node, Engine::NodeGraph::NodeBase)

#define SENDER_NODE_END(Name) \
    METATABLE_END(Name##Node) \
    SERIALIZETABLE_END(Name##Node)

#define DEFAULT_SENDER_NODE_BEGIN(Name, ...) SENDER_NODE_BEGIN(Name, Engine::NodeGraph::DefaultConfig, __VA_ARGS__)
#define CONSTANT_SENDER_NODE_BEGIN(Name, ...) SENDER_NODE_BEGIN(Name, Engine::NodeGraph::ConstantConfig, __VA_ARGS__)

#define ARGUMENT(Name, Index)                                \
    PROPERTY(Name, getArguments<Index>, setArguments<Index>) \
    ENCAPSULATED_FIELD(Name, getArguments<Index>, setArguments<Index>)
