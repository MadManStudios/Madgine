#include "Interfaces/interfaceslib.h"

#include "Generic/guard.h"
#include "Interfaces/debug/stacktrace.h"


#include <csignal>

#ifndef NDEBUG
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
    abort();
}

void madgine_signal_handler(int signal) {
    {
        std::stringstream cout;
        cout << "Signal caught: " << signal << " (Madgine - Handler)\n ";
        cout << "Stack-Trace:\n";
        for (const Engine::Debug::TraceBack &trace : Engine::Debug::StackTrace<64>::getCurrent(1).calculateReadable())
            cout << trace.mFunction << " (" << trace.mFile << ": " << trace.mLineNr << ")\n";
        LOG_FATAL(cout.str());
    }
    abort();
}

static Engine::Guard global { []() {
    std::set_terminate(&madgine_terminate_handler);
    std::signal(SIGSEGV, &madgine_signal_handler);
    std::signal(SIGILL, &madgine_signal_handler);
    std::signal(SIGABRT, &madgine_signal_handler);
    std::signal(SIGTERM, &madgine_signal_handler);
    std::signal(SIGFPE, &madgine_signal_handler);
    std::signal(SIGINT, &madgine_signal_handler);
} };
#endif