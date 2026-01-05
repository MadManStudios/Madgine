#include "Interfaces/interfaceslib.h"

#include <csignal>

#include "Generic/guard.h"

#include "Interfaces/debug/stacktrace.h"

#ifndef NDEBUG


static std::terminate_handler sOldTerminateHandler;
using signal_handler = decltype(std::signal(SIGSEGV, std::declval<void(*)(int)>()));
static signal_handler sOldSigSegvHandler;
static signal_handler sOldSigIllHandler;
static signal_handler sOldSigAbrtHandler;
static signal_handler sOldSigTermHandler;
static signal_handler sOldSigFpeHandler;
static signal_handler sOldSigIntHandler;

void finalize() {
    std::set_terminate(sOldTerminateHandler);
    std::signal(SIGSEGV, sOldSigSegvHandler);
    std::signal(SIGILL, sOldSigIllHandler);
    std::signal(SIGABRT, sOldSigAbrtHandler);
    std::signal(SIGTERM, sOldSigTermHandler);
    std::signal(SIGFPE, sOldSigFpeHandler);
    std::signal(SIGINT, sOldSigIntHandler);

    abort();
}

void madgine_terminate_handler()
{
    {
        std::stringstream cout;
        cout << "Terminate called! (Madgine-Handler)\n";
        cout << "Stack-Trace:\n";
        for (const Engine::Debug::TraceBack &trace : Engine::Debug::StackTrace<64>::getCurrent(1).calculateReadable())
            cout << trace.mFunction << " (" << trace.mFile << ": " << trace.mLineNr << ")\n";
        LOG_FATAL(cout.str());
    }

    finalize();
}

void madgine_signal_handler(int signal)
{
    {
        std::stringstream cout;
        cout << "Signal caught: " << signal << " (Madgine - Handler)\n ";
        cout << "Stack-Trace:\n";
        for (const Engine::Debug::TraceBack &trace : Engine::Debug::StackTrace<64>::getCurrent(1).calculateReadable())
            cout << trace.mFunction << " (" << trace.mFile << ": " << trace.mLineNr << ")\n";
        LOG_FATAL(cout.str());
    }

    finalize();
}

static Engine::Guard global { []() {
    sOldTerminateHandler = std::set_terminate(&madgine_terminate_handler);
    sOldSigSegvHandler = std::signal(SIGSEGV, &madgine_signal_handler);
    sOldSigIllHandler = std::signal(SIGILL, &madgine_signal_handler);
    sOldSigAbrtHandler = std::signal(SIGABRT, &madgine_signal_handler);
    sOldSigTermHandler = std::signal(SIGTERM, &madgine_signal_handler);
    sOldSigFpeHandler = std::signal(SIGFPE, &madgine_signal_handler);
    sOldSigIntHandler = std::signal(SIGINT, &madgine_signal_handler);
} };
#endif