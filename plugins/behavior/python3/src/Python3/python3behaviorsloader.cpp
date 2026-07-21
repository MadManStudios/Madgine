#include "python3lib.h"

#include "python3behaviorsloader.h"

#include "Meta/reflect/objectptr.h"
#include "Meta/reflect/value.h"

#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/behavior/behaviorhandle.h"

#include "Meta/reflect/metatable_impl.h"

#include "python3env.h"
#include "util/pyexecution.h"
#include "util/pyobjectutil.h"
#include "util/python3lock.h"
#include "util/pytype.h"

UNIQUECOMPONENT(Engine::Behavior::Python3::Python3BehaviorsLoader)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3BehaviorsLoader)
    FUNCTION(find_spec, name, import_path, target_module)
    FUNCTION(create_module, spec)
    FUNCTION(exec_module, module)
METATABLE_END(Engine::Behavior::Python3::Python3BehaviorsLoader)

METATABLE_BEGIN_BASE(Engine::Behavior::Python3::Python3BehaviorsLoader::Resource, Engine::Resources::ResourceBase)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3BehaviorsLoader::Resource)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3BehaviorsLoader::Handle)
// READONLY_PROPERTY(Data, dataPtr)
METATABLE_END(Engine::Behavior::Python3::Python3BehaviorsLoader::Handle)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *PyBehaviors_get(PyObject *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_ParseTuple(args, "s", &name))
                return NULL;

            auto it = BehaviorFactoryRegistry::sComponentsByName().find(name);
            if (it != BehaviorFactoryRegistry::sComponentsByName().end()) {
                std::string fullName = "Behaviors."s + name;
                return PyImport_ImportModule(fullName.c_str());
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %s!", name, PyModule_GetName(self));
            return NULL;
        }

        static PyMethodDef PyBehaviorsMethods[] = {
            { "__getattr__", PyBehaviors_get, METH_VARARGS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        static PyModuleDef PyBehaviors_module = {
            PyModuleDef_HEAD_INIT,
            "Engine", /* name of module */
            "Contains all registered behaviors", /* module documentation, may be NULL */
            -1, /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
            PyBehaviorsMethods
        };

        PyMODINIT_FUNC PyInit_Behaviors(void)
        {
            PyObject *m = PyModule_Create(&PyBehaviors_module);
            if (m == NULL)
                return NULL;

            return m;
        }

        struct PyBehavior {
            PyObject_HEAD
                Behavior mBehavior;
        };

        PyObject *PyBehavior_await(PyObject *self)
        {
            Behavior behavior = std::move(reinterpret_cast<PyBehavior *>(self)->mBehavior);

            return PyAwait(std::move(behavior));
        }

        PyAsyncMethods PyBehaviorAsyncMethods = {
            .am_await = PyBehavior_await
        };

        PyTypeObject PyBehaviorType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Behavior.Behavior",
            .tp_basicsize = sizeof(PyBehavior),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyBehavior, &PyBehavior::mBehavior>,
            .tp_as_async = &PyBehaviorAsyncMethods,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Behavior",
            .tp_new = PyType_GenericNew,
        };

        struct PyBehaviorHandle {
            PyObject_HEAD
                BehaviorHandle mHandle;
        };

        static PyObject *PyBehaviorHandle_call(PyBehaviorHandle *self, PyObject *args, PyObject *kwargs)
        {
            assert(PyTuple_Check(args));

            size_t argCount = PyTuple_Size(args);
            Reflect::ArgumentList arguments { std::true_type {}, argCount };

            for (size_t i = 0; i < argCount; ++i) {
                PYTHON3_PROPAGATE_ERROR(fromPyObject(arguments[i], PyTuple_GetItem(args, i)));
            }

            PyObject *obj = PyObject_CallObject((PyObject *)&PyBehaviorType, NULL);
            new (&reinterpret_cast<PyBehavior *>(obj)->mBehavior) Behavior(self->mHandle.create(arguments));

            return obj;
        }

        PyTypeObject PyBehaviorHandleType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Behavior.BehaviorHandle",
            .tp_basicsize = sizeof(PyBehaviorHandle),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyBehaviorHandle, &PyBehaviorHandle::mHandle>,
            .tp_call = (ternaryfunc)PyBehaviorHandle_call,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of BehaviorHandle",
            .tp_new = PyType_GenericNew,
        };

        PyObject *PyBehaviorsRegistry_get(PyObject *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_ParseTuple(args, "s", &name))
                return NULL;

            std::string_view moduleName = PyModule_GetName(self);
            assert(moduleName.starts_with("Behaviors."));
            moduleName = moduleName.substr(strlen("Behaviors."));

            auto it = BehaviorFactoryRegistry::sComponentsByName().find(moduleName);
            if (it != BehaviorFactoryRegistry::sComponentsByName().end()) {
                const BehaviorFactoryBase *factory = BehaviorFactoryRegistry::get(it->second).mFactory;

                if (std::ranges::contains(factory->names(), name)) {

                    PyObject *obj = PyObject_CallObject((PyObject *)&PyBehaviorHandleType, NULL);
                    new (&reinterpret_cast<PyBehaviorHandle *>(obj)->mHandle) BehaviorHandle(it->second, name);

                    [[maybe_unused]] int result = PyModule_AddObjectRef(self, name, obj);
                    assert(result == 0);

                    return obj;
                }
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %s!", name, PyModule_GetName(self));
            return NULL;
        }

        static PyMethodDef PyBehaviorsRegistryMethods[] = {
            { "__getattr__", PyBehaviorsRegistry_get, METH_VARARGS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        Python3BehaviorsLoader::Python3BehaviorsLoader()
            : ResourceLoader({}, {})
        {
        }

        void Python3BehaviorsLoader::setup()
        {
            [[maybe_unused]] auto result = PyList_Append(PyModulePtr { "sys" }.get("meta_path"), toPyObject(Reflect::ScopePtr { this }));
            assert(result == 0);
        }

        Threading::Task<bool> Python3BehaviorsLoader::loadImpl(PyModulePtr &module, ResourceDataInfo &info, Platform::Filesystem::FileEventType event)
        {
            throw 0;
        }

        void Python3BehaviorsLoader::unloadImpl(PyModulePtr &module)
        {
            throw 0;
        }

        Reflect::Result Python3BehaviorsLoader::find_spec(Reflect::Value &result, std::string_view name, std::optional<std::vector<std::string_view>> import_path, std::optional<Reflect::ObjectPtr> target_module)
        {
            if (!name.starts_with("Behaviors."))
                return {};
            Python3InnerLock lock;
            PyModulePtr module { "importlib.machinery" };
            if (!module) {
                return std::make_unique<Reflect::Error>(fetchError());
            }

            std::string_view strippedName = name.substr(strlen("Behaviors."));
            std::string_view registryName = strippedName.substr(0, strippedName.find('.'));

            PyObjectPtr spec = module.get("ModuleSpec").call("sO", name.data(), toPyObject(Reflect::ScopePtr { this }));
            return fromPyObject(result, spec);
        }

        Reflect::Result Python3BehaviorsLoader::create_module(Reflect::Value &result, Reflect::ObjectPtr spec)
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

            PyModuleDef *PyBehaviorsRegistry_module = new PyModuleDef {
                PyModuleDef_HEAD_INIT,
                PyBytes_AsString(asciiName), /* name of module */
                "test", /* module documentation, may be NULL */
                -1, /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
                PyBehaviorsRegistryMethods
            };

            return fromPyObject(result, PyModule_Create(PyBehaviorsRegistry_module));
        }

        Reflect::Result Python3BehaviorsLoader::exec_module(Reflect::Value &result, Reflect::ObjectPtr module)
        {
            return {};
        }

    }
}
}
