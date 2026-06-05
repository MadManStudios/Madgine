#pragma once

#include "Generic/fixed_string.h"

namespace Engine {

namespace Plugins {

    struct IniFile;
    struct IniSection;

    struct PluginManager;
    struct Plugin;
    struct PluginSection;

    struct BinaryInfo;

    struct IndexHolder;

    struct CollectorManager;
    template <typename T, typename _Collector, typename _Base>
    struct Component;
    template <typename Base>
    struct NamedComponent;
    template <typename Registry>
    struct Selector;
    template <typename C, typename Registry, typename Base>
    struct Container;
    template <typename T, typename _Collector, typename Base>
    struct VirtualComponentBase;
    template <typename T, typename Base, typename _VBase = Base>
    struct VirtualComponentImpl;
    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... Annotations>
    struct Registry;
    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... Annotations>
    struct NamedRegistry;
    template <typename Registry>
    struct Collector;

    struct RegistryBase;
    struct TypeInfo;
}

namespace Debug {

    struct TraceBack;
    struct StackTraceIterator;

    namespace Memory {
        struct StatsMemoryResource;
        struct MemoryTracker;
    }

    namespace Profiler {
        struct Profiler;
        struct ProfilerThread;
    }

    namespace Tasks {
        struct TaskTracker;
    }

    template <typename T, size_t S>
    struct History;
    template <typename T>
    struct HistoryData;
}

namespace Threading {
    struct TaskQueue;
    struct TaskHandle;
    template <typename T, bool Immediate = false>
    struct Task;
    template <typename T>
    using ImmediateTask = Task<T, true>;

    struct TaskPromiseBase;

    struct WorkGroup;
    struct WorkGroupHandle;
    struct Scheduler;

    struct DataMutex;

    struct CustomClock;
    struct CustomTimepoint;
}

}
