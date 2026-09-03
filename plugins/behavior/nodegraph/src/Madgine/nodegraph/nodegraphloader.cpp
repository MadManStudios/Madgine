#include "../nodegraphlib.h"

#include "nodegraphloader.h"

#include "Meta/serialize/streams/streamresult.h"

#include "Madgine/behavior/behaviordescriptor.h"
#include "Madgine/behavior/typedparametertuple.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/type/storageops_impl.h"

#include "nodeinterpreter.h"

RESOURCELOADER(Engine::Behavior::NodeGraph::NodeGraphLoader)

BEHAVIOR_FACTORY(NodeGraph, Engine::Behavior::NodeGraph::NodeGraphBehaviorFactory)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        Resources::with_handle_t::sender<NodeInterpreterSender, NodeGraphLoader::Handle> NodeGraphLoader::Handle::interpret(ParameterTuple args) const
        {
            return NodeInterpreterSender { *this, std::move(args) } | Resources::with_handle(Handle { *this });
        }

        NodeGraphLoader::NodeGraphLoader()
            : ResourceLoader({ ".ngp" }, { .mAutoReload = true, .mIconName = "NodeGraph.png" })
        {
        }

        Threading::Task<bool> NodeGraphLoader::loadImpl(NodeGraph &graph, ResourceDataInfo &info)
        {
            Serialize::StreamResult result = co_await graph.loadFromFile(info.resource()->path());

            if (result.mState != Serialize::StreamState::OK) {
                LOG_ERROR("Error loading Nodegraph (" << info.resource()->path() << "):\n"
                                                      << result);
                co_return false;
            }

            co_return true;
        }

        void NodeGraphLoader::unloadImpl(NodeGraph &graph)
        {
        }

        std::vector<std::string_view> NodeGraphBehaviorFactory::names() const
        {
            const auto &names = NodeGraphLoader::getSingleton().resources() | std::ranges::views::transform([](Resources::ResourceBase *resource) { return resource->name(); });
            return { names.begin(), names.end() };
        }

        UniqueOpaquePtr NodeGraphBehaviorFactory::load(std::string_view name) const
        {
            UniqueOpaquePtr ptr;
            ptr.setupAs<NodeGraphLoader::Handle>() = NodeGraphLoader::load(name);
            return ptr;
        }

        Threading::TaskFuture<bool> NodeGraphBehaviorFactory::state(const UniqueOpaquePtr &handle) const
        {
            return handle.as<NodeGraphLoader::Handle>().info()->loadingTask();
        }

        void NodeGraphBehaviorFactory::release(UniqueOpaquePtr &ptr) const
        {
            ptr.release<NodeGraphLoader::Handle>();
        }

        std::string_view NodeGraphBehaviorFactory::name(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return graph.name();
        }

        Behavior NodeGraphBehaviorFactory::create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return graph.interpret(args);
        }

        ParameterTuple NodeGraphBehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return ParameterTuple { std::make_tuple(), auto_pack<> {} };
        }

        const BehaviorDescriptor &NodeGraphBehaviorFactory::descriptor(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            throw "TODO";
            return {};
        }

    }
}
}
