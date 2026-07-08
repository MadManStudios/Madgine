#pragma once

#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *PyNamespace_get(PyObject *self, PyObject *args);

        struct MADGINE_PYTHON3_EXPORT Python3NamespaceLoader : Resources::ResourceLoader<Python3NamespaceLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3NamespaceLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3NamespaceLoader();

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
