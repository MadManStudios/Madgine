#pragma once

#include "Meta/reflect/objectinstance.h"

#include "pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct PyObjectInstance : Reflect::ObjectInstance {

            PyObjectInstance(PyObject *obj);
            ~PyObjectInstance();

            Reflect::Result getValue(Reflect::Value &retVal, std::string_view name) const override;
            void setValue(std::string_view name, const Reflect::Value &value) override;
            std::map<std::string_view, Reflect::Value> values() const override;

            Reflect::Result call(Reflect::Value &retVal, const Reflect::ArgumentList &args) override;

            std::string descriptor() const override;

            PyObject *get() const;

            PyObjectPtr mPtr;
        };

    }
}
}