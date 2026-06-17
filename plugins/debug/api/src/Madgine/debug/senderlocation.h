#pragma once

#include "Generic/callable_view.h"
#include "Generic/closure.h"
#include "statedescriptor.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT SenderLocation {
        template <typename T>
        using helper = void(const T &);
        using CB = typename Execution::StateTypes::prepend<Execution::State::Value>::template transform<helper>::template instantiate<CallableView>;
        SenderLocation(Closure<void(CB)> state);

        void stepInto(Debug::SenderLocation *&location, ContextInfo &context);
        void stepOut(Debug::SenderLocation *&location, ContextInfo &context);

        void visit(CB visitor) const;

        Closure<void(CB)> mState;
    };

}
}