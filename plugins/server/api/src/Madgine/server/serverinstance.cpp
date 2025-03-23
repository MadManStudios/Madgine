#include "../serverlib.h"

#include "serverinstance.h"

namespace Engine {
namespace Server {
    size_t ServerInstance::sInstanceCounter = 0;

    ServerInstance::ServerInstance(std::string_view name, Closure<int()> entrypoint)
        : mName(name.empty() ? "thread_"s + std::to_string(++sInstanceCounter) : name)
        , mWorkGroup(mName, std::move(entrypoint))
    {
    }

    const char *ServerInstance::key() const
    {
        return mName.c_str();
    }

}
}