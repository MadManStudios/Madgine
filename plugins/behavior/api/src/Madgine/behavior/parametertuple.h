#pragma once

#include "Meta/reflect/type.h"
#include "Meta/serialize/streams/streamresult.h"

namespace Engine {
namespace Behavior {

    template <typename... Ty>
    struct ParameterTupleInstance;

    template <typename Names, typename... Ty>
    struct TypedParameterTupleInstance;

    struct ParameterTupleBase {
        virtual size_t size() const = 0;
        virtual std::string_view name(size_t index) const = 0;
        virtual Reflect::ExtendedType type(size_t index) const = 0;

        virtual std::unique_ptr<ParameterTupleBase> clone() = 0;
        virtual Reflect::ScopePtr customScopePtr() = 0;

        virtual Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in) = 0;
        virtual void write(Serialize::CallerHierarchyFormattedSerializeStream out) = 0;
        virtual Serialize::StreamResult applyMap(Serialize::CallerHierarchyFormattedSerializeStream in, bool success) = 0;

        virtual ~ParameterTupleBase() = default;
    };

    struct MADGINE_BEHAVIOR_EXPORT ParameterTuple {

        ParameterTuple() = default;
        ParameterTuple(const ParameterTuple &other)
            : mTuple(other.mTuple->clone())
        {
        }
        ParameterTuple(ParameterTuple &&) noexcept = default;

        ParameterTuple(std::unique_ptr<ParameterTupleBase> tuple);

        template <typename... Ty, auto... Names>
        ParameterTuple(std::tuple<Ty...> parameters, auto_pack<Names...>)
            : mTuple(std::make_unique<TypedParameterTupleInstance<auto_pack<Names...>, Ty...>>(std::move(parameters)))
        {
        }

        ParameterTuple &operator=(const ParameterTuple &other)
        {
            mTuple = other.mTuple->clone();
            return *this;
        }

        ParameterTuple &operator=(ParameterTuple &&) noexcept = default;

        Reflect::ScopePtr customScopePtr();

        template <typename... Ty>
        bool get(std::tuple<Ty...> &out) const
        {
            ParameterTupleInstance<Ty...> *instance = dynamic_cast<ParameterTupleInstance<Ty...> *>(mTuple.get());
            if (instance) {
                out = instance->mTuple;
            }
            return instance;
        }

        template <typename T>
        const T &get() const
        {
            return *static_cast<T *>(mTuple.get());
        }

        size_t size() const
        {
            return mTuple->size();
        }

        std::string_view name(size_t index) const
        {
            return mTuple->name(index);
        }

        Reflect::ExtendedType type(size_t index) const
        {
            return mTuple->type(index);
        }

        void reset()
        {
            mTuple.reset();
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mTuple);
        }

    private:
        friend struct Serialize::Operations<ParameterTuple>;

        MADGINE_BEHAVIOR_EXPORT friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, ParameterTuple &tuple, Serialize::CallerHierarchyFormattedSerializeStream in, bool success);

        template <typename... Configs>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, ParameterTuple &tuple, bool active, bool existenceChanged, const CallerHierarchyBasePtr &)
        {
        }

        std::unique_ptr<ParameterTupleBase> mTuple;
    };
}

namespace Serialize {
    template <>
    struct MADGINE_BEHAVIOR_EXPORT Operations<Behavior::ParameterTuple> {
        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Behavior::ParameterTuple &tuple, const char *name);
        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Behavior::ParameterTuple &tuple, const char *name);

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };
}

}
