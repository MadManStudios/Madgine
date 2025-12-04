#pragma once

#include "Generic/functor.h"
#include "Generic/genericresult.h"

#include "../filesystem/path.h"

namespace Engine {
namespace Dl {

    ENUM_BASE(DlAPIResult, GenericResult,
        DEPENDENCY_ERROR)

    struct INTERFACES_EXPORT DlHandle {
        DlHandle() = default;
        DlHandle(const DlHandle &) = delete;
        ~DlHandle();

        DlHandle &operator=(const DlHandle &) = delete;

        DlAPIResult open(std::string_view name);
        DlAPIResult close();

        explicit operator bool() const { return mHandle; }

        const void *getSymbol(std::string_view name) const;
        Filesystem::Path fullPath(std::string_view symbolName) const;

    private:
        void *mHandle = nullptr;
    };

}
}