#include "python3lib.h"

#include "python3fileloader.h"

#include "Generic/execution/algorithm.h"

#include "Meta/reflect/value.h"

#include "Meta/reflect/metatable_impl.h"

#include "python3env.h"
#include "util/pydictptr.h"
#include "util/pymoduleptr.h"
#include "util/pyobjectutil.h"
#include "util/python3lock.h"

UNIQUECOMPONENT(Engine::Behavior::Python3::Python3FileLoader)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3FileLoader)
    MEMBER(mResources)
    FUNCTION(find_spec, name, import_path, target_module)
    FUNCTION(create_module, spec)
    FUNCTION(exec_module, module)
    FUNCTION(get_source, name)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader)

METATABLE_BEGIN_BASE(Engine::Behavior::Python3::Python3FileLoader::Resource, Engine::Resources::ResourceBase)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader::Resource)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3FileLoader::Handle)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3FileLoader::Handle)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3FileLoader::Python3FileLoader()
            : ResourceLoader({ ".py" }, { .mAutoLoad = true, .mInplaceReload = true, .mIconName = "Python-logo.png" })
        {
        }

        void Python3FileLoader::setup()
        {
            [[maybe_unused]] auto result = PyList_Append(PyModulePtr { "sys" }.get("meta_path"), toPyObject(Reflect::ScopePtr { this }));
            assert(result == 0);
        }

        void Python3FileLoader::cleanup()
        {
            mTables.clear();
        }

        Threading::Task<bool> Python3FileLoader::loadImpl(PyModulePtr &module, ResourceDataInfo &info, Platform::Filesystem::FileEventType event)
        {
            if (!co_await Python3Environment::getSingleton().state())
                co_return false;

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
            PythonFunctionInfo info = functionInfo(fn);

            auto transformed = info.mArguments | std::views::transform([](const PythonFunctionArgument &arg) {
                return Reflect::FunctionArgument { arg.mType, arg.mName };
            });
            mArgumentsHolder = { transformed.begin(), transformed.end() };

            mArguments = mArgumentsHolder.data();
            mArgumentsCount = mArgumentsHolder.size();

            mIsMemberFunction = false;

            mName = info.mName;

            mFunctionPtr = [](const FunctionTable *self, Reflect::Value &retVal, const Reflect::ArgumentList &args) {
                Python3InnerLock lock;
                return fromPyObject(retVal, static_cast<const Python3FunctionTable *>(self)->mFunctionObject.call(args));
            };

            Reflect::__Reflect_impl__::registerFunction(*this);
        }

        Python3FileLoader::Python3FunctionTable::~Python3FunctionTable()
        {
            Reflect::__Reflect_impl__::unregisterFunction(*this);
        }

        Reflect::Result Python3FileLoader::find_spec(Reflect::Value &result, std::string_view name, std::optional<std::vector<std::string_view>> import_path, std::optional<Reflect::ObjectPtr> target_module)
        {
            Resource *res = get(name, this);
            if (!res)
                return {};
            Python3InnerLock lock;
            PyModulePtr module { "importlib.machinery" };
            if (!module) {
                return std::make_unique<Reflect::Error>(fetchError());
            }
            PyObjectPtr spec = module.get("ModuleSpec").call({ { { "loader_state", toPyObject(Reflect::ScopePtr { res }) } } }, "sO", res->name().data(), toPyObject(Reflect::ScopePtr { this }));
            return fromPyObject(result, spec);
        }

        Reflect::Result Python3FileLoader::create_module(Reflect::Value &result, Reflect::ObjectPtr spec)
        {
            Python3InnerLock lock;
            Reflect::Value resourcePtr;
            REFLECT_PROPAGATE_ERROR(fromPyObject(resourcePtr, PyObjectPtr { toPyObject(spec) }.get("loader_state")));
            Resource *res = scope_cast<Resource>(resourcePtr.as<Reflect::ScopePtr>());
            Handle handle = create(res, Platform::Filesystem::FileEventType::FILE_CREATED, this);
            handle.info()->setPersistent(true);
            PyModulePtr &module = *getDataPtr(handle, this, false);
            assert(!module);
            module = PyModulePtr::create(res->name());
            return fromPyObject(result, module);
        }

        Reflect::Result Python3FileLoader::exec_module(Reflect::Value &result, Reflect::ObjectPtr module)
        {
            Python3InnerLock lock;

            PyObjectPtr moduleObject { toPyObject(module) };
            Reflect::Value resourcePtr;
            REFLECT_PROPAGATE_ERROR(fromPyObject(resourcePtr, moduleObject.get("__spec__").get("loader_state")));
            Resource *res = scope_cast<Resource>(resourcePtr.as<Reflect::ScopePtr>());

            std::string sourceCode = res->readAsText();
            PyObjectPtr code = Py_CompileString(sourceCode.c_str(), res->path().c_str(), Py_file_input);
            if (!code) {
                return std::make_unique<Reflect::Error>(fetchError());
            }

            PyObject *dict = PyModule_GetDict(moduleObject);

            if (!PyDict_Contains(dict, PyUnicode_FromString("__builtins__"))) {
                PyDict_SetItemString(dict, "__builtins__", PyEval_GetBuiltins());
            }

            PyObjectPtr status = PyEval_EvalCode(code, dict, dict);
            if (!status) {
                return std::make_unique<Reflect::Error>(fetchError());
            }
            
            PyObject *key, *value = NULL;
            Py_ssize_t pos = 0;

            while (PyDict_Next(dict, &pos, &key, &value)) {
                if (PyFunction_Check(value)) {
                    mTables.emplace_back(PyObjectPtr::fromBorrowed(value));
                }
            }
            return {};
        }

        Reflect::Result Python3FileLoader::get_source(Reflect::Value &result, std::string_view name)
        {
            Resource *res = get(name, this);
            if (!res)
                return {};
            toValue(result, res->readAsText());
            return {};
        }

        PythonFunctionInfo Python3FileLoader::functionInfo(PyObject *fn)
        {

            PyObjectPtr signature = PyModulePtr { "inspect" }.get("signature").call("(O)", (PyObject *)fn);

            PyObjectPtr name = PyObject_GetAttrString(fn, "__name__");
            PyObjectPtr ascii_name = PyUnicode_AsASCIIString(name);

            PythonFunctionInfo result { PyBytes_AsString(ascii_name), PyToValueTypeDesc(signature.get("return_annotation")) };

            PyObjectPtr parameters = signature.get("parameters");            

            PyObjectPtr iter = PyObject_GetIter(parameters);

            while (PyObjectPtr key = PyIter_Next(iter)) {
                PyObjectPtr parameter = PyObject_GetItem(parameters, key);
                PyObjectPtr type = parameter.get("annotation");

                PyObjectPtr ascii = PyUnicode_AsASCIIString(key);


                if (Py_IS_TYPE(type, &Py_GenericAliasType)) {
                    type = PyTuple_GetItem(type.get("__args__"), 0);                    
                    result.mArguments.push_back({ PyBytes_AsString(ascii), Reflect::ExtendedType { Reflect::ExtendedTypeEnum::VariantType, { Reflect::toType<std::monostate>(), PyToValueTypeDesc(type) } }, Reflect::AccessorFlags_Named });
                } else {
                    result.mArguments.push_back({ PyBytes_AsString(ascii), PyToValueTypeDesc(type) });
                }

                
            }

            return result;
        }

    }
}
}
