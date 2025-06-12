#pragma once

#include "Generic/functor.h"

#include "Interfaces/filesystem/path.h"

namespace Engine {
namespace Render {

    struct ShaderMetadata {
        Filesystem::Path mPath;
        std::vector<Filesystem::Path> mIncludePaths;
    };
    
    struct MADGINE_RENDER_EXPORT ShaderObjectPtr {

        ShaderObjectPtr() = default;

        template <typename O>
            requires(std::is_reference_v<O> && std::derived_from<std::remove_reference_t<O>, ShaderObjectBase>)
        ShaderObjectPtr(O &&object)
            : mObject { &object, NoOpFunctor {} }
        {
        }

        template <typename O>
            requires(!std::is_reference_v<O>)
        ShaderObjectPtr(O &&object)
            : mObject { std::make_shared<O>(std::move(object)) }
        {
        }

        const ShaderObjectBase *operator->() const;

        MADGINE_RENDER_EXPORT friend std::ostream &operator<<(std::ostream &out, const ShaderObjectPtr &p);

        explicit constexpr operator bool() const;

        std::shared_ptr<const ShaderObjectBase> mObject;
    };


    struct MADGINE_RENDER_EXPORT ShaderObjectBase {

        ShaderObjectBase(std::vector<ShaderObjectPtr> dependencies);

        void generate() const;

        std::chrono::file_clock::time_point timestamp() const;       
        std::chrono::file_clock::time_point chainTimestamp() const;

        std::string name() const;        

        virtual std::string entrypoint() const = 0;
        virtual const ShaderMetadata &metadata() const = 0;
        virtual void toHLSL(std::ostream &o) const = 0;
        

    protected:
        std::vector<ShaderObjectPtr> mDependencies;
    };

    template <typename R, typename... Args>
    struct ShaderObject : ShaderObjectBase {
        using ShaderObjectBase::ShaderObjectBase;
    };

}
}