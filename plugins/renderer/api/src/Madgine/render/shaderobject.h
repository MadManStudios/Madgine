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

        explicit operator bool() const;

        std::shared_ptr<const ShaderObjectBase> mObject;
    };

    struct MADGINE_RENDER_EXPORT ShaderObjectBase {

        ShaderObjectBase(std::vector<ShaderObjectPtr> dependencies);

        virtual void generate() const;

        std::chrono::file_clock::time_point timestamp() const;
        std::chrono::file_clock::time_point chainTimestamp() const;

        std::string name() const;

        virtual std::string entrypoint() const = 0;
        virtual const ShaderMetadata &metadata() const = 0;
        virtual void toHLSL(std::ostream &o) const = 0;

    protected:
        std::vector<ShaderObjectPtr> mDependencies;
    };

    template <fixed_string R, fixed_string In, typename... ConstantBuffers>
    struct TypedShaderObjectPtr;

    template <typename Base, fixed_string R, fixed_string In, typename... ConstantBuffers>
    struct ShaderObject : Base {
        using Base::Base;

        template <typename Other>
        auto operator|(Other &&other) const &
        {
            return TypedShaderObjectPtr<R, In, ConstantBuffers...> { *this } | std::forward<Other>(other);
        }
    };

    struct MADGINE_RENDER_EXPORT MergedShaderObjectBase : ShaderObjectBase {
        MergedShaderObjectBase(const ShaderObjectPtr &first, const ShaderObjectPtr &second);

        std::string entrypoint() const override;
        const ShaderMetadata &metadata() const override;
        void toHLSLImpl(std::ostream &o, std::string_view r, std::string_view in) const;

    private:
        ShaderMetadata mMetadata;
    };

    template <fixed_string R, fixed_string In, typename... ConstantBuffers>
    struct MergedShaderObject : ShaderObject<MergedShaderObjectBase, R, In, ConstantBuffers...> {
        using ShaderObject<MergedShaderObjectBase, R, In, ConstantBuffers...>::ShaderObject;
        void toHLSL(std::ostream& o) const override {
            MergedShaderObjectBase::toHLSLImpl(o, R, In);
        }
    };

    template <fixed_string R, fixed_string In, typename... ConstantBuffers>
    struct TypedShaderObjectPtr : ShaderObjectPtr {

        template <typename T1, typename T2>
        using F = typename std::conditional_t<std::is_void_v<T1>, std::type_identity<T2>, std::enable_if<OneOf<T2, T1, void>, T1>>::type;

        template <fixed_string R2, fixed_string In2>
        struct helper {
            template <typename... T>
            using type = MergedShaderObject<R2, In2, T...>;
        };

        template <typename Base, fixed_string R2, fixed_string In2, typename... ConstantBuffers2>
        auto operator|(const ShaderObject<Base, R2, In2, ConstantBuffers2...> &o)
        {
            return operator|(TypedShaderObjectPtr<R2, In2, ConstantBuffers2...> { o });
        }

        template <fixed_string R2, typename... ConstantBuffers2>
        auto operator|(const TypedShaderObjectPtr<R2, R, ConstantBuffers2...> &o)
        {

            constexpr size_t size = std::max(sizeof...(ConstantBuffers), sizeof...(ConstantBuffers2));
            using combinedBuffers = typename type_pack<ConstantBuffers...>::template resize<size>::template zip<F, typename type_pack<ConstantBuffers2...>::template resize<size>>;

            return typename combinedBuffers::template instantiate<helper<R2, In>::template type> { *this, o };
        }
    };

}
}