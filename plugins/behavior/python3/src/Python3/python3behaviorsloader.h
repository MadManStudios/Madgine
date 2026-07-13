#pragma once

#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyBehaviorType;
        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyBehaviorHandleType;
                
        PyMODINIT_FUNC PyInit_Behaviors(void);

        struct MADGINE_PYTHON3_EXPORT Python3BehaviorsLoader : Resources::ResourceLoader<Python3BehaviorsLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3BehaviorsLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3BehaviorsLoader();

            void setup();

            Threading::Task<bool> loadImpl(PyModulePtr &module, ResourceDataInfo &info, Platform::Filesystem::FileEventType event);
            void unloadImpl(PyModulePtr &module);

            Reflect::Result find_spec(Reflect::Value &result, std::string_view name, std::optional<std::vector<std::string_view>> import_path, std::optional<Reflect::ObjectPtr> target_module);

            Reflect::Result create_module(Reflect::Value &result, Reflect::ObjectPtr spec);
            Reflect::Result exec_module(Reflect::Value &result, Reflect::ObjectPtr module);
        };

    }
}
}
