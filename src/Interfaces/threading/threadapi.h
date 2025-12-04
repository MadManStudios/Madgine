#pragma once

namespace Engine {
namespace Threading {

    INTERFACES_EXPORT void setCurrentThreadName(const std::string &name);
    INTERFACES_EXPORT std::string getCurrentThreadName();

}
}
