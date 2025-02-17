#pragma once

#include "Madgine/resources/resourceloader.h"

#include "util/pymoduleptr.h"

#include "Meta/keyvalue/objectptr.h"

#include "Meta/keyvalue/functiontable.h"

#include "Madgine/behaviorcollector.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3FileLoader : Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>> {

            using Base = Resources::ResourceLoader<Python3FileLoader, PyModulePtr, std::list<Placeholder<0>>>;

            Python3FileLoader();

            void setup();
            void cleanup();

            Threading::Task<bool> loadImpl(PyModulePtr &module, ResourceDataInfo &info, Filesystem::FileEventType event);
            void unloadImpl(PyModulePtr &module);

            void find_spec(ValueType &result, std::string_view name, std::optional<std::string_view> import_path, ObjectPtr target_module);

            void create_module(ValueType &result, ObjectPtr spec);
            void exec_module(ValueType &result, ObjectPtr module);

        private:
            struct Python3FunctionTable : FunctionTable{
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
            Threading::TaskFuture<ParameterTuple> createParameters(const UniqueOpaquePtr &handle) const override;
            ParameterTuple createDummyParameters(const UniqueOpaquePtr &handle) const override;
            std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const override;
            std::vector<BindingDescriptor> bindings(const UniqueOpaquePtr &handle) const override;
            size_t subBehaviorCount(const UniqueOpaquePtr &handle) const override;
        };

    }
}
}

DECLARE_BEHAVIOR_FACTORY(Engine::Scripting::Python3::Python3BehaviorFactory)


REGISTER_TYPE(Engine::Scripting::Python3::Python3FileLoader)