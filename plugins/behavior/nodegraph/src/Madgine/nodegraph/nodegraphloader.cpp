#include "../nodegraphlib.h"

#include "nodegraphloader.h"

#include "Meta/serialize/streams/streamresult.h"

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

        Resources::with_handle_t::sender<NodeInterpreterSender, NodeGraphLoader::Handle> NodeGraphLoader::Handle::interpret() const
        {
            return NodeInterpreterSender { *this } | Resources::with_handle(Handle { *this });
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
            return graph.interpret();
        }

        ParameterTuple NodeGraphBehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return ParameterTuple { std::make_tuple(), auto_pack<> {} };
        }

        std::vector<Reflect::ExtendedType> NodeGraphBehaviorFactory::parameterTypes(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return {};
        }

        std::vector<Reflect::ExtendedType> NodeGraphBehaviorFactory::resultTypes(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            return {};
        }

        std::vector<NamedDescriptor> NodeGraphBehaviorFactory::namedInputs(const UniqueOpaquePtr &handle) const
        {
            const NodeGraphLoader::Handle &graph = handle.as<NodeGraphLoader::Handle>();
            auto bindings = graph->mNamedInputs | std::views::transform(&NodeGraph::NamedInput::mDescriptor);
            return { bindings.begin(), bindings.end() };
        }

        size_t NodeGraphBehaviorFactory::subBehaviorCount(const UniqueOpaquePtr &handle) const
        {
            return 0;
        }

    }
}
}
