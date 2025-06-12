#pragma once

#include "shaderobject.h"

namespace Engine {
namespace Render {

    template <typename R, typename... Args>
    struct ShaderFileObject : ShaderObject<R, Args...> {

        ShaderFileObject(const ShaderMetadata &metadata, std::string_view entrypoint, std::vector<ShaderObjectPtr> dependencies)
            : ShaderObject<R, Args...>(std::move(dependencies))
            , mMetadata(metadata)
            , mEntrypoint(entrypoint)
        {
        }

        std::string entrypoint() const override
        {
            return mEntrypoint;
        }
        const ShaderMetadata &metadata() const override
        {
            return mMetadata;
        }
        void toHLSL(std::ostream &o) const override
        {
            throw 0;
        }

        const ShaderMetadata &mMetadata;
        std::string mEntrypoint;
    };

}
}