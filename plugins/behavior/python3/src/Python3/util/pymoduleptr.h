#pragma once

#include "pyobjectptr.h"
#include "pydictptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {
    
        struct MADGINE_PYTHON3_EXPORT PyModulePtr : PyObjectPtr {
            PyModulePtr() = default;
            PyModulePtr(std::string_view name);

            static PyModulePtr fromBorrowed(PyObject *);

            using PyObjectPtr::operator=;

            static PyModulePtr create(std::string_view name);

            PyDictPtr getDict() const;
        };
    
    }
}
}