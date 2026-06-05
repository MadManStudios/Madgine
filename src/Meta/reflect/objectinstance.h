#pragma once

#include "result.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT ObjectInstance {
        virtual ~ObjectInstance() = default;

        virtual Result getValue(Value &retVal, std::string_view name) const = 0;
        virtual void setValue(std::string_view name, const Value &value) = 0;
        virtual std::map<std::string_view, Value> values() const;

        virtual Result call(Value &retVal, const ArgumentList &args);

        virtual std::string descriptor() const;
    };

}
}