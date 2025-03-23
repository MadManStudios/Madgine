#pragma once

#include "Modules/threading/workgrouphandle.h"

#include "Generic/closure.h"

namespace Engine {
namespace Server {
    struct MADGINE_SERVER_EXPORT ServerInstance {

        ServerInstance(std::string_view name, Closure<int()> callback);
        ServerInstance(const ServerInstance &) = delete;

        const char *key() const;

        //ValueType toValueType() const;

    private:
        std::string mName;
        static size_t sInstanceCounter;

        Threading::WorkGroupHandle mWorkGroup;
    };
}
}
