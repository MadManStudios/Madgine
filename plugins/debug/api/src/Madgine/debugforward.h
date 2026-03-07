#pragma once

namespace Engine {
namespace Debug {

    struct Debugger;
    struct ContextInfo;
    struct DebugListener;
    struct SenderLocation;

    enum class ContinuationMode;

    enum class ContinuationType;

    template <auto...>
    struct DebuggableLifetime;

    struct DebuggableLifetimeBase;

}
}