#include "../interfaceslib.h"

#if WINDOWS

#include "processapi.h"

#define NOMINMAX
#include <Windows.h>

namespace Engine {
namespace Process {

    struct ProcessAuxiliaryData {
        ~ProcessAuxiliaryData() {
            CloseHandle(mProcessInfo.hProcess);
            CloseHandle(mProcessInfo.hThread);

            CloseHandle(mStdoutPipe);
            CloseHandle(mStderrPipe);
        }

        PROCESS_INFORMATION mProcessInfo {};
        HANDLE mWaitHandle;

        HANDLE mStdoutPipe;
        HANDLE mStderrPipe;
    };

    ProcessState::ProcessState(std::string executable, std::vector<std::string> commandLine, std::chrono::milliseconds timeout)
        : mData(std::make_unique<ProcessAuxiliaryData>())
        , mExecutable(std::move(executable))
        , mCommandLine(std::move(commandLine))
        , mTimeout(timeout)
    {
    }

    ProcessState::~ProcessState() = default;



    void ProcessState::start() {

        SECURITY_ATTRIBUTES sa {};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = true;

        HANDLE out_write;
        HANDLE err_write;

        bool result = CreatePipe(&mData->mStdoutPipe, &out_write, &sa, 0);
        assert(result);

        result = SetHandleInformation(mData->mStdoutPipe, HANDLE_FLAG_INHERIT, 0);
        assert(result);

        result = CreatePipe(&mData->mStderrPipe, &err_write, &sa, 0);
        assert(result);

        result = SetHandleInformation(mData->mStderrPipe, HANDLE_FLAG_INHERIT, 0);
        assert(result);

        STARTUPINFO startup {};
        startup.cb = sizeof(STARTUPINFO);
        startup.hStdOutput = out_write;
        startup.hStdError = err_write;
        startup.dwFlags |= STARTF_USESTDHANDLES;

        std::string command = mExecutable;
        for (const std::string &argument : mCommandLine)
            command += " " + argument;

        result = CreateProcess(nullptr, command.data(), nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &mData->mProcessInfo);
        assert(result);

        CloseHandle(out_write);
        CloseHandle(err_write);

        result = RegisterWaitForSingleObject(&mData->mWaitHandle, mData->mProcessInfo.hProcess, [](void *self, BOOLEAN timedOut) { 
            ProcessState *state = static_cast<ProcessState *>(self);

            UnregisterWait(state->mData->mWaitHandle);

            std::string stdOut;
            std::string stdErr;

            DWORD dwRead;
            CHAR chBuf[512];

            for (;;) {
                bool result = ReadFile(state->mData->mStdoutPipe, chBuf, 512, &dwRead, NULL);
                if (!result || dwRead == 0)
                    break;
                stdOut.append(chBuf, dwRead);
            } 
            for (;;) {
                bool result = ReadFile(state->mData->mStderrPipe, chBuf, 512, &dwRead, NULL);
                if (!result || dwRead == 0)
                    break;
                stdErr.append(chBuf, dwRead);
            } 

            if (timedOut) {
                TerminateProcess(state->mData->mProcessInfo.hProcess, -1);
                state->set_error(GenericResult::UNKNOWN_ERROR);
            }
            else {
                DWORD exit_code;
                if (!GetExitCodeProcess(state->mData->mProcessInfo.hProcess, &exit_code))
                    state->set_error(GenericResult::UNKNOWN_ERROR);
                else 
                    state->set_value(exit_code, std::move(stdOut), std::move(stdErr));
            }
        },
            this, mTimeout.count(), WT_EXECUTEINWAITTHREAD);
        assert(result);
    }

}
}

#endif