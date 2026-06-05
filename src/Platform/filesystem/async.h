#pragma once

#include "Generic/execution/virtualsender.h"

#include "asyncoperations.h"

namespace Engine {
namespace Platform {
    namespace Filesystem {

        PLATFORM_EXPORT void checkAsyncIOCompletion();
        PLATFORM_EXPORT void cancelAllAsyncIO();
        PLATFORM_EXPORT size_t pendingIOOperationCount();

        inline auto readFileAsync(const Path &path)
        {
            return Execution::make_virtual_sender<AsyncFileReadState>(path);
        }
        using AsyncFileRead = std::invoke_result_t<decltype(readFileAsync), Path>;

    }
}
}