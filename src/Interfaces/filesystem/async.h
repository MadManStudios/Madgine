#pragma once

#include "asyncoperations.h"
#include "Generic/execution/virtualsender.h"

namespace Engine {
namespace Filesystem {

    INTERFACES_EXPORT void checkAsyncIOCompletion();
    INTERFACES_EXPORT void cancelAllAsyncIO();
    INTERFACES_EXPORT size_t pendingIOOperationCount();

    inline auto readFileAsync(const Path& path) {
        return Execution::make_virtual_sender<AsyncFileReadState>(path);
    }
    using AsyncFileRead = std::invoke_result_t<decltype(readFileAsync), Path>;

}
}