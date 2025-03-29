#include "../python3lib.h"

#include "pymoduleptr.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Scripting::Python3::PyModulePtr)
METATABLE_END(Engine::Scripting::Python3::PyModulePtr)

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyModulePtr::PyModulePtr(std::string_view name)
        {
            PyObject *modules = PyImport_GetModuleDict();
            mObject = PyDict_GetItemString(modules, name.data());
            if (mObject) {
                Py_INCREF(mObject);
            } else {
                mObject = PyImport_ImportModule(name.data());
            }
        }

        
        PyModulePtr PyModulePtr::fromBorrowed(PyObject *module)
        {
            Py_INCREF(module);
            PyModulePtr ptr;
            ptr.mObject = module;
            return ptr;
        }

        PyModulePtr PyModulePtr::create(std::string_view name)
        {
            PyModulePtr module;
            module.mObject = PyModule_NewObject(PyUnicode_FromString(name.data()));
            return module;
        }

        PyDictPtr PyModulePtr::getDict() const
        {
            return PyDictPtr::fromBorrowed(PyModule_GetDict(mObject));
        }

    }
}
}