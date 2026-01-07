#pragma once

#include "shaderobject.h"

namespace Engine {
namespace Render {

    struct ShaderFileObjectBase : ShaderObjectBase {
        ShaderFileObjectBase(const ShaderMetadata &metadata, std::string_view entrypoint, std::vector<ShaderObjectPtr> dependencies)
            : ShaderObjectBase(std::move(dependencies))
            , mMetadata(metadata)
            , mEntrypoint(entrypoint)
        {
        }

        void generate() const override {

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

    template <fixed_string R, fixed_string In, typename... ConstantBuffers>
    using ShaderFileObject = ShaderObject<ShaderFileObjectBase, R, In, ConstantBuffers...>;        

}
}