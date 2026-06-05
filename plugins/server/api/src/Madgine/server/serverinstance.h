#pragma once

#include "Generic/closure.h"

#include "Modules/threading/workgrouphandle.h"

namespace Engine {
namespace Core {
    struct MADGINE_SERVER_EXPORT ServerInstance {

        ServerInstance(std::string_view name, Closure<int()> callback);
        ServerInstance(const ServerInstance &) = delete;

        const char *key() const;

        // ValueType toValueType() const;

    private:
        std::string mName;
        static size_t sInstanceCounter;

        Threading::WorkGroupHandle mWorkGroup;
    };
}
}
