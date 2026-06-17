#include "../debuglib.h"

#include "statedescriptor.h"

namespace Engine {
namespace Execution {
    namespace State {

        void Value::get(Reflect::Value& out) const {
            mGet(out, mPtr);
        }

        void Value::set(const Reflect::Value &in) const
        {
            mSet(mPtr, in);
        }

    }
}
}