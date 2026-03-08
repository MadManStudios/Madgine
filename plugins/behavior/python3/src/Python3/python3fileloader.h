#pragma once

#include "Meta/keyvalue/functiontable.h"
#include "Meta/keyvalue/objectptr.h"

#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3FileLoader : Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3FileLoader();

            void setup();
            void cleanup();

            Threading::Task<bool> loadImpl(PyModulePtr &module, ResourceDataInfo &info, Filesystem::FileEventType event);
            void unloadImpl(PyModulePtr &module);

            KeyValueResult find_spec(ValueType &result, std::string_view name, std::optional<std::string_view> import_path, ObjectPtr target_module);

            KeyValueResult create_module(ValueType &result, ObjectPtr spec);
            KeyValueResult exec_module(ValueType &result, ObjectPtr module);

        private:
            struct Python3FunctionTable : FunctionTable {
                Python3FunctionTable(PyObjectPtr fn);
                ~Python3FunctionTable();

                std::vector<FunctionArgument> mArgumentsHolder;
                std::vector<std::string> mArgumentsNames;
                std::string mNameHolder;

                PyObjectPtr mFunctionObject;
            };
            std::list<Python3FunctionTable> mTables;
        };

        struct Python3BehaviorFactory : BehaviorFactory<Python3BehaviorFactory> {
            std::vector<std::string_view> names() const override;
            UniqueOpaquePtr load(std::string_view name) const override;
            Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const override;
            void release(UniqueOpaquePtr &ptr) const override;
            std::string_view name(const UniqueOpaquePtr &handle) const override;
            Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const override;
            ParameterTuple createParameters(const UniqueOpaquePtr &handle) const override;
            std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<NamedDescriptor> namedInputs(const UniqueOpaquePtr &handle) const override;
            size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;
        };

    }
}
}
