#include "python3lib.h"

#include "python3fileloader.h"

#include <iostream>

#include "Generic/execution/algorithm.h"

#include "Meta/keyvalue/valuetype.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/parametertuple.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "python3env.h"
#include "util/pydictptr.h"
#include "util/pyexecution.h"
#include "util/pymoduleptr.h"
#include "util/pyobjectutil.h"
#include "util/python3lock.h"

UNIQUECOMPONENT(Engine::Behavior::Python3::Python3FileLoader)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3FileLoader)
    MEMBER(mResources)
    FUNCTION(find_spec, name, import_path, target_module)
    FUNCTION(create_module, spec)
    FUNCTION(exec_module, module)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader)

METATABLE_BEGIN_BASE(Engine::Behavior::Python3::Python3FileLoader::Resource, Engine::Resources::ResourceBase)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader::Resource)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3FileLoader::Handle)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader::Handle)

BEHAVIOR_FACTORY(Python3, Engine::Behavior::Python3::Python3BehaviorFactory)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3FileLoader::Python3FileLoader()
            : ResourceLoader({ ".py" }, { .mAutoLoad = true, .mInplaceReload = true, .mIconName = "Python-logo.png" })
        {
        }

        void Python3FileLoader::setup()
        {
            auto result = PyList_Append(PyModulePtr { "sys" }.get("meta_path"), toPyObject(ScopePtr { this }));
            assert(result == 0);
        }

        void Python3FileLoader::cleanup()
        {
            mTables.clear();
        }

        Threading::Task<bool> Python3FileLoader::loadImpl(PyModulePtr &module, ResourceDataInfo &info, Filesystem::FileEventType event)
        {
            Python3Lock lock {};

            if (!module) {
                module = { info.resource()->name() };
                if (!module) {
                    LOG_ERROR(fetchError());
                    co_return false;
                }
            } else {
                PyModulePtr { "importlib" }.call("reload", "(O)", (PyObject *)module);
            }

            co_return true;
        }

        void Python3FileLoader::unloadImpl(PyModulePtr &module)
        {
            Python3Lock lock {};
            module.reset();
        }

        Python3FileLoader::Python3FunctionTable::Python3FunctionTable(PyObjectPtr fn)
            : mFunctionObject(fn)
        {

            PyObjectPtr signature = PyModulePtr { "inspect" }.get("signature").call("(O)", (PyObject *)fn);
            PyObjectPtr parameters = signature.get("parameters");

            PyObject *key = NULL, *value = NULL;
            Py_ssize_t pos = 0;

            PyObjectPtr iter = PyObject_GetIter(parameters);

            while (PyObjectPtr key = PyIter_Next(iter)) {
                PyObjectPtr parameter = PyObject_GetItem(parameters, key);
                PyObjectPtr type = parameter.get("annotation");

                PyObjectPtr ascii = PyUnicode_AsASCIIString(key);
                mArgumentsNames.emplace_back(PyBytes_AsString(ascii));
                mArgumentsHolder.push_back({ PyToValueTypeDesc(type), mArgumentsNames.back() });
            }
            mArguments = mArgumentsHolder.data();
            mArgumentsCount = mArgumentsHolder.size();

            mIsMemberFunction = false;

            mReturnType = PyToValueTypeDesc(signature.get("return_annotation"));
            PyObjectPtr name = fn.get("__name__");
            PyObjectPtr ascii_name = PyUnicode_AsASCIIString(name);
            mNameHolder = PyBytes_AsString(ascii_name);
            mName = mNameHolder;

            mFunctionPtr = [](const FunctionTable *self, ValueType &retVal, const ArgumentList &args) {
                Python3InnerLock lock;
                return fromPyObject(retVal, static_cast<const Python3FunctionTable *>(self)->mFunctionObject.call(args));
            };

            registerFunction(*this);
        }

        Python3FileLoader::Python3FunctionTable::~Python3FunctionTable()
        {
            unregisterFunction(*this);
        }

        KeyValueResult Python3FileLoader::find_spec(ValueType &result, std::string_view name, std::optional<std::string_view> import_path, ObjectPtr target_module)
        {
            Resource *res = get(name, this);
            if (!res)
                return {};
            Python3InnerLock lock;
            PyModulePtr module { "importlib.machinery" };
            if (!module) {
                return { GenericResult::UNKNOWN_ERROR, std::make_unique<KeyValueError>(fetchError()) };
            }
            PyObjectPtr spec = module.get("ModuleSpec").call({ { { "loader_state", toPyObject(ScopePtr { res }) } } }, "sO", res->name().data(), toPyObject(ScopePtr { this }));
            return fromPyObject(result, spec);
        }

        KeyValueResult Python3FileLoader::create_module(ValueType &result, ObjectPtr spec)
        {
            Python3InnerLock lock;
            ValueType resourcePtr;
            KEYVALUE_PROPAGATE_ERROR(fromPyObject(resourcePtr, PyObjectPtr { toPyObject(spec) }.get("loader_state")));
            Resource *res = scope_cast<Resource>(resourcePtr.as<ScopePtr>());
            Handle handle = create(res, Filesystem::FileEventType::FILE_CREATED, this);
            handle.info()->setPersistent(true);
            PyModulePtr &module = *getDataPtr(handle, this, false);
            assert(!module);
            module = PyModulePtr::create(res->name());
            return fromPyObject(result, module);
        }

        KeyValueResult Python3FileLoader::exec_module(ValueType &result, ObjectPtr module)
        {
            Python3InnerLock lock;

            PyObjectPtr moduleObject { toPyObject(module) };
            ValueType resourcePtr;
            KEYVALUE_PROPAGATE_ERROR(fromPyObject(resourcePtr, moduleObject.get("__spec__").get("loader_state")));
            Resource *res = scope_cast<Resource>(resourcePtr.as<ScopePtr>());

            PyModulePtr importlib { "importlib.util" };

            PyObjectPtr spec = importlib.get("spec_from_file_location").call("ss", res->name().data(), res->path().c_str());

            KEYVALUE_PROPAGATE_ERROR(fromPyObject(result, spec.get("loader").get("exec_module").call("(O)", (PyObject *)moduleObject)));

            PyObject *dict = PyModule_GetDict(moduleObject);

            PyObject *key, *value = NULL;
            Py_ssize_t pos = 0;

            while (PyDict_Next(dict, &pos, &key, &value)) {
                if (PyFunction_Check(value)) {
                    Python3FunctionTable &table = mTables.emplace_back(PyObjectPtr::fromBorrowed(value));
                }
            }
            return {};
        }

        std::vector<std::string_view> Python3BehaviorFactory::names() const
        {
            const auto &names = Python3FileLoader::getSingleton().resources() | std::ranges::views::transform([](Resources::ResourceBase *resource) { return resource->name(); });
            return { names.begin(), names.end() };
        }

        UniqueOpaquePtr Python3BehaviorFactory::load(std::string_view name) const
        {
            UniqueOpaquePtr ptr;
            ptr.setupAs<Python3FileLoader::Handle>() = Python3FileLoader::load(name);
            return ptr;
        }

        Threading::TaskFuture<bool> Python3BehaviorFactory::state(const UniqueOpaquePtr &handle) const
        {
            return handle.as<Python3FileLoader::Handle>().info()->loadingTask();
        }

        void Python3BehaviorFactory::release(UniqueOpaquePtr &ptr) const
        {
            ptr.release<Python3FileLoader::Handle>();
        }

        std::string_view Python3BehaviorFactory::name(const UniqueOpaquePtr &handle) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return file.name();
        }

        Behavior Python3BehaviorFactory::create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            Python3Lock lock;
            PyObjectPtr main = file->get("main");
            if (!main)
                return Execution::just_error(fetchError());
            return main.callAsync();
        }

        ParameterTuple Python3BehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return ParameterTuple { std::make_tuple(), auto_pack<> {} };
        }

        std::vector<ValueTypeDesc> Python3BehaviorFactory::parameterTypes(const UniqueOpaquePtr &handle) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return {};
        }

        std::vector<ValueTypeDesc> Python3BehaviorFactory::resultTypes(const UniqueOpaquePtr &handle) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return {};
        }

        std::vector<NamedDescriptor> Python3BehaviorFactory::namedInputs(const UniqueOpaquePtr &handle) const
        {
            const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return {};
        }

        size_t Python3BehaviorFactory::subBehaviorCount(const UniqueOpaquePtr &handle) const
        {
            return 0;
        }

    }
}
}
