#include "../serverlib.h"
#include "serverlog.h"

#include <iostream>

#if WINDOWS
#    include <conio.h>

#    define NOMINMAX
#    include <Windows.h>
#    undef NO_ERROR
#endif

namespace Engine {
namespace Server {
    ServerLog::ServerLog(const std::string &name)
        : StandardLog(name)
        , mConsole(false)
    {
    }

    ServerLog::~ServerLog()
    {
        stopConsole();
    }

    void ServerLog::log(std::string_view msg, Engine::Log::MessageType lvl, const char *file, size_t line)
    {
        std::cout << "\r";
        StandardLog::log(msg, lvl, file, line);
        std::cout << "prompt> " << mCurrentCmd;
        std::cout.flush();
    }

#if WINDOWS
    static DWORD fdwOldMode;
#endif

    bool ServerLog::startConsole()
    {
        std::cout << "prompt> ";
        std::cout.flush();

#if WINDOWS
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
        std::string cmd;

        bool console = GetConsoleMode(input, &fdwOldMode);
        // disable mouse and window input
        DWORD fdwMode = fdwOldMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
        console &= static_cast<bool>(SetConsoleMode(input, fdwMode));

        mConsole = console;
#    else
        mConsole = true;
#    endif
#else
        mConsole = true;
#endif

        return true;
    }

    void ServerLog::stopConsole()
    {
#if WINDOWS
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(input, fdwOldMode);
#    endif
#endif
    }

    std::vector<std::string> ServerLog::update()
    {
        std::vector<std::string> cmds;

#if WINDOWS
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)

        CHAR buffer[256];

        DWORD dwRead = 0;

        HANDLE input = GetStdHandle(STD_INPUT_HANDLE);

        if (mConsole) {
            if (WaitForSingleObject(input, 0) == WAIT_OBJECT_0) {
                DWORD count;
                bool result = GetNumberOfConsoleInputEvents(input, &count);
                assert(result);
                if (count > 0) {
                    INPUT_RECORD record[100];
                    result = ReadConsoleInput(input, record, 100, &count);
                    assert(result);

                    for (DWORD i = 0; i < count; ++i) {
                        if (record[i].EventType == KEY_EVENT && record[i].Event.KeyEvent.bKeyDown) {
                            char c = record[i].Event.KeyEvent.uChar.AsciiChar;
                            bool send = false;
                            switch (c) {
                            case '\r':
                            case '\n':
                                send = true;
                                break;
                            default:
                                mCurrentCmd += c;
                                std::cout << c;
                                break;
                            }

                            if (send && !mCurrentCmd.empty()) {
                                cmds.emplace_back(std::move(mCurrentCmd));
                                mCurrentCmd.clear();
                                std::cout << std::endl;
                                std::cout << "prompt> ";
                                std::cout.flush();
                            }
                        }
                    }
                }
            }
        } else {
            bool result = PeekNamedPipe(input, nullptr, 0, nullptr, &dwRead, nullptr);
            assert(result);

            if (dwRead > 0) {
                result = ReadFile(input, buffer, std::min(static_cast<DWORD>(sizeof buffer), dwRead), &dwRead, nullptr);
                assert(result && dwRead > 0);
                for (DWORD i = 0; i < dwRead; ++i) {
                    char c = buffer[i];
                    bool send = false;
                    switch (c) {
                    case '\r':
                    case '\n':
                        send = true;
                        break;
                    default:
                        mCurrentCmd += c;
                        break;
                    }

                    if (send && !mCurrentCmd.empty()) {
                        cmds.emplace_back(std::move(mCurrentCmd));
                        mCurrentCmd.clear();
                        std::cout << "prompt> ";
                        std::cout.flush();
                    }
                }
            }
        }
#    endif
#endif
        return cmds;
    }
}
}
