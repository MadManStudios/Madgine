#include "python3lib.h"

#include "python3namespaceloader.h"

#include "Meta/reflect/objectptr.h"
#include "Meta/reflect/value.h"

#include "Meta/reflect/metatable_impl.h"

#include "python3env.h"
#include "util/pyobjectutil.h"
#include "util/python3lock.h"
#include "util/pytype.h"

UNIQUECOMPONENT(Engine::Behavior::Python3::Python3NamespaceLoader)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3NamespaceLoader)
    FUNCTION(find_spec, name, import_path, target_module)
    FUNCTION(create_module, spec)
    FUNCTION(exec_module, module)
METATABLE_END(Engine::Behavior::Python3::Python3NamespaceLoader)

METATABLE_BEGIN_BASE(Engine::Behavior::Python3::Python3NamespaceLoader::Resource, Engine::Resources::ResourceBase)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3NamespaceLoader::Resource)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3NamespaceLoader::Handle)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3NamespaceLoader::Handle)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyMethodDef PyNamespaceMethods[] = {
            { "__getattr__", PyNamespace_get, METH_VARARGS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        PyObject *PyNamespace_get(PyObject *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_ParseTuple(args, "s", &name))
                return NULL;

            std::string fullName = std::string { PyModule_GetName(self) } + "." + name;

            auto typeName = Type::resolveTypeName(fullName, ".");
            if (typeName) {
                if (typeName->mStorageOps) {
                    PyObject *type = PyObject_CallObject((PyObject *)&PyTypeType, NULL);
                    if (!type)
                        return NULL;
                    reinterpret_cast<PyType *>(type)->mType = typeName;
                    [[maybe_unused]] int result = PyModule_AddObjectRef(self, name, type);
                    assert(result == 0);
                    return type;
                } else {
                    return PyImport_ImportModule(fullName.c_str());
                }
            }
            
            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %s!", name, PyModule_GetName(self));
            return NULL;
        }

        Python3NamespaceLoader::Python3NamespaceLoader()
            : ResourceLoader({}, {})
        {
        }

        void Python3NamespaceLoader::setup()
        {
            [[maybe_unused]] auto result = PyList_Append(PyModulePtr { "sys" }.get("meta_path"), toPyObject(Reflect::ScopePtr { this }));
            assert(result == 0);
        }

        Threading::Task<bool> Python3NamespaceLoader::loadImpl(PyModulePtr &module, ResourceDataInfo &info, Platform::Filesystem::FileEventType event)
        {
            throw 0;
        }

        void Python3NamespaceLoader::unloadImpl(PyModulePtr &module)
        {
            throw 0;
        }

        Reflect::Result Python3NamespaceLoader::find_spec(Reflect::Value &result, std::string_view name, std::optional<std::vector<std::string_view>> import_path, std::optional<Reflect::ObjectPtr> target_module)
        {
            if (!name.starts_with("Engine"))
                return {};
            Python3InnerLock lock;
            PyModulePtr module { "importlib.machinery" };
            if (!module) {
                return std::make_unique<Reflect::Error>(fetchError());
            }

            //TODO: Existence check

            PyObjectPtr spec = module.get("ModuleSpec").call("sO", name.data(), toPyObject(Reflect::ScopePtr { this }));
            return fromPyObject(result, spec);
        }

        Reflect::Result Python3NamespaceLoader::create_module(Reflect::Value &result, Reflect::ObjectPtr spec)
        {
            Python3InnerLock lock;

            PyObjectPtr specObject = toPyObject(spec);
            if (!specObject)
                return std::make_unique<Reflect::Error>(fetchError());

            PyObjectPtr name = specObject.get("name");
            if (!name)
                return std::make_unique<Reflect::Error>(fetchError());

            PyObjectPtr asciiName = PyUnicode_AsASCIIString(name);
            if (!asciiName)
                return std::make_unique<Reflect::Error>(fetchError());

            PyModuleDef *PyNamespace_module = new PyModuleDef {
                PyModuleDef_HEAD_INIT,
                PyBytes_AsString(asciiName), /* name of module */
                "test", /* module documentation, may be NULL */
                -1, /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
                PyNamespaceMethods
            };

            PyObject *module = PyModule_Create(PyNamespace_module);

            PyModule_AddObject(module, "__path__", PyList_New(0));

            return fromPyObject(result, module);
        }

        Reflect::Result Python3NamespaceLoader::exec_module(Reflect::Value &result, Reflect::ObjectPtr module)
        {
            return {};
        }

    }
}
}
