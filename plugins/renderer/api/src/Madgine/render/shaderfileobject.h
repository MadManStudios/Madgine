#pragma once

#include "shaderobject.h"

#include "Madgine/render/resourceblock.h"

namespace Engine {
namespace Render {

    struct ShaderFileObjectBase : ShaderObjectBase {
        ShaderFileObjectBase(const ShaderMetadata &metadata, std::string_view entrypoint, std::vector<ShaderObjectPtr> dependencies, PipelineSignature signature)
            : ShaderObjectBase(std::move(dependencies))
            , mMetadata(metadata)
            , mEntrypoint(entrypoint)
            , mSignature(std::move(signature))
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
        PipelineSignature signature() const override
        {
            return mSignature;
        }
        void toHLSL(std::ostream &o) const override
        {
            throw 0;
        }

        const ShaderMetadata &mMetadata;
        std::string mEntrypoint;
        PipelineSignature mSignature;
    };    

    template <fixed_string R, fixed_string In, typename... ConstantBuffers>
    using ShaderFileObject = ShaderObject<ShaderFileObjectBase, R, In, ConstantBuffers...>;        

}
}