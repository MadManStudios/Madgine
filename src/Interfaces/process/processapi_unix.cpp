#include "../interfaceslib.h"

#if UNIX

#    include <spawn.h>
#    include <sys/wait.h>
#    include <unistd.h>

#    include "processapi.h"

namespace Engine {
namespace Process {

    static std::mutex sMutex;
    static std::vector<ProcessState *> sPendingProcesses;

    struct ProcessAuxiliaryData {

        ~ProcessAuxiliaryData()
        {
            if (mStdoutPipe != -1)
                close(mStdoutPipe);
            if (mStderrPipe != -1)
                close(mStderrPipe);
        }

        pid_t mPid;
        int mStdoutPipe = -1;
        int mStderrPipe = -1;
    };

    ProcessState::ProcessState(std::string executable, std::vector<std::string> commandLine, std::chrono::milliseconds timeout)
        : mData(std::make_unique<ProcessAuxiliaryData>())
        , mExecutable(std::move(executable))
        , mCommandLine(std::move(commandLine))
        , mTimeout(timeout)
    {
    }

    ProcessState::~ProcessState() = default;

    bool check(ProcessState *state)
    {
        int status;
        int result = waitpid(state->mData->mPid, &status, WNOHANG);
        if (result == -1) {
            state->set_error(GenericResult::UNKNOWN_ERROR);
            return true;
        }

        if (result != 0) {

            std::string stdOut;
            std::string stdErr;

            char buffer[512];
            while (ssize_t bytes_read = read(state->mData->mStdoutPipe, buffer, 512)) {
                if (bytes_read < 0)
                    break;
                stdOut.append(buffer, bytes_read);
            }
            while (ssize_t bytes_read = read(state->mData->mStderrPipe, buffer, 512)) {
                if (bytes_read < 0)
                    break;
                stdErr.append(buffer, bytes_read);
            }

            state->set_value(WIFEXITED(status), std::move(stdOut), std::move(stdErr));
            return true;
        }
        return false;
    }

    void ProcessState::start()
    {
        std::vector<char *> commandLine;
        commandLine.push_back(mExecutable.data());
        std::transform(mCommandLine.begin(), mCommandLine.end(), std::back_inserter(commandLine), [](std::string &s) { return s.data(); });
        commandLine.push_back(nullptr);

        int stdoutPipe[2];
        int stderrPipe[2];

        if (pipe(stdoutPipe) == -1) {
            set_error(GenericResult::UNKNOWN_ERROR);
            return;
        }

        if (pipe(stderrPipe) == -1) {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
            set_error(GenericResult::UNKNOWN_ERROR);
            return;
        }

        posix_spawn_file_actions_t actions;

        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addclose(&actions, stdoutPipe[0]);
        posix_spawn_file_actions_addclose(&actions, stderrPipe[0]);

        posix_spawn_file_actions_adddup2(&actions, stdoutPipe[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, stderrPipe[1], STDERR_FILENO);

        posix_spawn_file_actions_addclose(&actions, stdoutPipe[1]);
        posix_spawn_file_actions_addclose(&actions, stderrPipe[1]);

        int result = posix_spawn(&mData->mPid, mExecutable.c_str(), &actions, nullptr, commandLine.data(), nullptr);
        posix_spawn_file_actions_destroy(&actions);

        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        mData->mStdoutPipe = stdoutPipe[0];
        mData->mStderrPipe = stderrPipe[0];

        if (result != 0) {
            set_error(GenericResult::UNKNOWN_ERROR);
            return;
        }

        if (!check(this)) {
            std::unique_lock lock { sMutex };
            sPendingProcesses.push_back(this);
        }
    }

    void checkAsyncProcessCompletion()
    {
        std::unique_lock lock { sMutex };
        std::erase_if(sPendingProcesses, check);
    }

    size_t pendingProcesses()
    {
        std::unique_lock lock { sMutex };
        return sPendingProcesses.size();
    }

    void execute(std::string_view command)
    {
        throw 0;
    }

}
}

#endif