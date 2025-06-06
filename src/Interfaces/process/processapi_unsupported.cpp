#include "../interfaceslib.h"

#if !WINDOWS

#    include "processapi.h"

namespace Engine {
namespace Process {

    struct ProcessAuxiliaryData {
    };

    ProcessState::ProcessState(std::string executable, std::vector<std::string> commandLine, std::chrono::milliseconds timeout)
        : mExecutable(std::move(executable))
        , mCommandLine(std::move(commandLine))
        , mTimeout(timeout)
    {
    }

    ProcessState::~ProcessState() = default;

    void ProcessState::start()
    {
        set_error(GenericResult::UNKNOWN_ERROR);
    }

}
}

#endif