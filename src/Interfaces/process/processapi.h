#pragma once

#include "Generic/execution/virtualstate.h"
#include "Generic/execution/virtualsender.h"

#include "Generic/genericresult.h"

namespace Engine {
namespace Process {

	
    struct ProcessAuxiliaryData;

    struct INTERFACES_EXPORT ProcessState : Execution::VirtualReceiverBase<GenericResult, int, std::string, std::string> {

        ProcessState(std::string executable, std::vector<std::string> commandLine, std::chrono::milliseconds timeout);
        ProcessState(const ProcessState &) = delete;
        ProcessState(ProcessState &&) = delete;
        ~ProcessState();

        ProcessState &operator=(const ProcessState &) = delete;
        ProcessState &operator=(ProcessState &&) = delete;

        void start();

        std::unique_ptr<ProcessAuxiliaryData> mData;

        std::string mExecutable;
        std::vector<std::string> mCommandLine;
        std::chrono::milliseconds mTimeout;
    };

    
    inline auto runAsync(std::string executable, std::vector<std::string> commandLine, std::chrono::milliseconds timeout = std::chrono::milliseconds::max())
    {
        return Execution::make_virtual_sender<ProcessState>(std::move(executable), std::move(commandLine), timeout);
    }


}
}