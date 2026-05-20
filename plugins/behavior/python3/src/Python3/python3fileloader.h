#pragma once

#include "Meta/keyvalue/functionargument.h"
#include "Meta/keyvalue/functiontable.h"
#include "Meta/keyvalue/objectptr.h"

#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct PythonFunctionArgument {
            std::string mName;
            ExtendedValueTypeDesc mType;
            AccessorFlags mFlags;
        };

        struct PythonFunctionInfo {
            std::string mName;
            ExtendedValueTypeDesc mReturnType;
            std::vector<PythonFunctionArgument> mArguments;
        };

        struct MADGINE_PYTHON3_EXPORT Python3FileLoader : Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3FileLoader();

            void setup();
            void cleanup();

            Threading::Task<bool> loadImpl(PyModulePtr &module, ResourceDataInfo &info, Filesystem::FileEventType event);
            void unloadImpl(PyModulePtr &module);

            KeyValueResult find_spec(ValueType &result, std::string_view name, std::optional<std::string_view> import_path, std::optional<ObjectPtr> target_module);

            KeyValueResult create_module(ValueType &result, ObjectPtr spec);
            KeyValueResult exec_module(ValueType &result, ObjectPtr module);

            static PythonFunctionInfo functionInfo(PyObject *fn);

        private:
            struct Python3FunctionTable : FunctionTable {
                Python3FunctionTable(PyObjectPtr fn);
                ~Python3FunctionTable();

                std::vector<FunctionArgument> mArgumentsHolder;

                PyObjectPtr mFunctionObject;
            };

            std::list<Python3FunctionTable> mTables;
        };

    }
}
}
