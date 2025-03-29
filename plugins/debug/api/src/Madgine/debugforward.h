#pragma once

namespace Engine {
namespace Debug {

    struct Debugger;
    struct ContextInfo;
    struct DebugLocation;
    struct ParentLocation;
    struct DebugListener;

    enum class ContinuationMode;

    enum class ContinuationType;

    template <auto...>
    struct DebuggableLifetime;

    struct DebuggableLifetimeBase;

}
}