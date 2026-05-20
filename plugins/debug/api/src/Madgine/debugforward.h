#pragma once

namespace Engine {
namespace Debug {

    struct Debugger;
    struct ContextInfo;
    struct DebugListener;
    struct SenderLocation;
    struct Continuation;

    enum class ContinuationMode;

    enum class ContinuationType;

    template <auto...>
    struct DebuggableLifetime;

    struct DebuggableLifetimeBase;

}
}