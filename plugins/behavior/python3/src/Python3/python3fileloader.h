#pragma once

#include "Meta/reflect/functionargument.h"
#include "Meta/reflect/functiontable.h"
#include "Meta/reflect/objectptr.h"

#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct PythonFunctionArgument {
            std::string mName;
            Reflect::ExtendedType mType;
            Reflect::AccessorFlags mFlags;
        };

        struct PythonFunctionInfo {
            std::string mName;
            Reflect::ExtendedType mReturnType;
            std::vector<PythonFunctionArgument> mArguments;
        };

        struct MADGINE_PYTHON3_EXPORT Python3FileLoader : Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3FileLoader();

            void setup();
            void cleanup();

            Threading::Task<bool> loadImpl(PyModulePtr &module, ResourceDataInfo &info, Platform::Filesystem::FileEventType event);
            void unloadImpl(PyModulePtr &module);

            Reflect::Result find_spec(Reflect::Value &result, std::string_view name, std::optional<std::vector<std::string_view>> import_path, std::optional<Reflect::ObjectPtr> target_module);

            Reflect::Result create_module(Reflect::Value &result, Reflect::ObjectPtr spec);
            Reflect::Result exec_module(Reflect::Value &result, Reflect::ObjectPtr module);

            Reflect::Result get_source(Reflect::Value &result, std::string_view name);

            static PythonFunctionInfo functionInfo(PyObject *fn);

        private:
            struct Python3FunctionTable : Reflect::FunctionTable {
                Python3FunctionTable(PyObjectPtr fn);
                ~Python3FunctionTable();

                std::vector<Reflect::FunctionArgument> mArgumentsHolder;

                PyObjectPtr mFunctionObject;
            };

            std::list<Python3FunctionTable> mTables;
        };

    }
}
}
