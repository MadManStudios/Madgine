#include "python3lib.h"

#include "python3env.h"

#include "Generic/cowstring.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Madgine/root/keyvalueregistry.h"
#include "Madgine/root/root.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "python3behaviors.h"
#include "python3fileloader.h"
#include "python3streamredirect.h"
#include "util/math/pymatrix3.h"
#include "util/math/pymatrix4.h"
#include "util/math/pyquaternion.h"
#include "util/math/pyvector2.h"
#include "util/math/pyvector3.h"
#include "util/math/pyvector4.h"
#include "util/pyapifunction.h"
#include "util/pybinding.h"
#include "util/pyboundapifunction.h"
#include "util/pydictptr.h"
#include "util/pyenum.h"
#include "util/pyflags.h"
#include "util/pyframeptr.h"
#include "util/pylistptr.h"
#include "util/pyobjectutil.h"
#include "util/pyownedscopeptr.h"
#include "util/pyscopeiterator.h"
#include "util/pyscopeptr.h"
#include "util/pysender.h"
#include "util/python3lock.h"
#include "util/pytype.h"
#include "util/pyvirtualiterator.h"
#include "util/pyvirtualrange.h"

#if PY_MINOR_VERSION < 11
#    include <frameobject.h>
#else
#    define Py_BUILD_CORE
#    include "internal/pycore_frame.h"
#endif

UNIQUECOMPONENT(Engine::Behavior::Python3::Python3Environment)

METATABLE_BEGIN(Engine::Behavior::Python3::Python3Environment)
METATABLE_END(Engine::Behavior::Python3::Python3Environment)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        static PyObject *
        PyEnvironment_get(PyObject *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_ParseTuple(args, "s", &name))
                return NULL;

            auto it = KeyValueRegistry::globals().find(name);
            if (it != KeyValueRegistry::globals().end()) {
                return toPyObject(it->second);
            }

            auto it2 = KeyValueRegistry::workgroupLocals().find(name);
            if (it2 != KeyValueRegistry::workgroupLocals().end()) {
                return toPyObject(it2->second);
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
            return NULL;
        }

        static PyObject *
        PyEnvironment_dir(PyObject *self, PyObject *args)
        {
            PyObject *list = PyList_New(KeyValueRegistry::globals().size() + KeyValueRegistry::workgroupLocals().size());
            size_t i = 0;
            for (std::string_view key : kvKeys(KeyValueRegistry::globals())) {
                PyList_SetItem(list, i++, toPyObject(key));
            }
            for (std::string_view key : kvKeys(KeyValueRegistry::workgroupLocals())) {
                PyList_SetItem(list, i++, toPyObject(key));
            }

            return list;
        }

        static PyMethodDef PyEnvironmentMethods[] = {
            { "__getattr__", PyEnvironment_get, METH_VARARGS, "" },
            { "__dir__", PyEnvironment_dir, METH_NOARGS, "List all Environment globals" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        static PyModuleDef PyEnv_module = {
            PyModuleDef_HEAD_INIT,
            "Environment", /* name of module */
            "test", /* module documentation, may be NULL */
            -1, /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
            PyEnvironmentMethods
        };

        static PyObject *PyNamespace_get(PyObject *self, PyObject *args);

        static PyMethodDef PyNamespaceMethods[] = {
            { "__getattr__", PyNamespace_get, METH_VARARGS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        static PyObject *PyNamespace_get(PyObject *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_ParseTuple(args, "s", &name))
                return NULL;

            std::string fullName = std::string { PyModule_GetName(self) } + "::" + name;
            std::string namespaceName = fullName + "::";

            const MetaTable *list = sTypeList();
            while (list) {

                if (fullName == list->mTypeName) {
                    PyObject *type = PyObject_CallObject((PyObject *)&PyTypeType, NULL);
                    if (!type)
                        return NULL;
                    reinterpret_cast<PyType *>(type)->mType = list;
                    int result = PyModule_AddObjectRef(self, name, type);
                    assert(result == 0);
                    return type;
                } else if (StringUtil::startsWith(list->mTypeName, namespaceName)) {
                    PyModuleDef PyNamespace_module = {
                        PyModuleDef_HEAD_INIT,
                        fullName.c_str(), /* name of module */
                        "test", /* module documentation, may be NULL */
                        -1, /* size of per-interpreter state of the module,
                             or -1 if the module keeps state in global variables. */
                        PyNamespaceMethods
                    };
                    PyObject *module = PyModule_Create(&PyNamespace_module);
                    if (!module)
                        return NULL;
                    int result = PyModule_AddObjectRef(self, name, module);
                    assert(result == 0);
                    return module;
                }

                list = list->mNext;
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %s!", name, PyModule_GetName(self));
            return NULL;
        }

        static PyMethodDef PyEngineMethods[] = {
            { "__getattr__", PyNamespace_get, METH_VARARGS, "" },
            { "Behavior", (PyCFunction)PyEngine_Behavior_decorator, METH_FASTCALL, "Decorator for Behavior functions" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        static PyModuleDef PyEngine_module = {
            PyModuleDef_HEAD_INIT,
            "Engine", /* name of module */
            "Wrappers for engine C++ classes", /* module documentation, may be NULL */
            -1, /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
            PyEngineMethods
        };

        PyMODINIT_FUNC PyInit_Engine(void)
        {

            if (PyType_Ready(&PyTypedScopePtrType) < 0)
                return NULL;
            if (PyType_Ready(&PyOwnedScopePtrType) < 0)
                return NULL;
            if (PyType_Ready(&PyApiFunctionType) < 0)
                return NULL;
            if (PyType_Ready(&PyBoundApiFunctionType) < 0)
                return NULL;
            if (PyType_Ready(&PyScopeIteratorType) < 0)
                return NULL;
            if (PyType_Ready(&PyVirtualSequenceRangeType) < 0)
                return NULL;
            if (PyType_Ready(&PyVirtualAssociativeRangeType) < 0)
                return NULL;
            if (PyType_Ready(&PyVirtualSequenceIteratorType) < 0)
                return NULL;
            if (PyType_Ready(&PyVirtualAssociativeIteratorType) < 0)
                return NULL;
            if (PyType_Ready(&PyVector2Type) < 0)
                return NULL;
            if (PyType_Ready(&PyVector3Type) < 0)
                return NULL;
            if (PyType_Ready(&PyVector4Type) < 0)
                return NULL;
            if (PyType_Ready(&PyMatrix3Type) < 0)
                return NULL;
            if (PyType_Ready(&PyMatrix4Type) < 0)
                return NULL;
            if (PyType_Ready(&PyQuaternionType) < 0)
                return NULL;
            if (PyType_Ready(&PySenderType) < 0)
                return NULL;
            if (PyType_Ready(&PySenderStateType) < 0)
                return NULL;
            if (PyType_Ready(&PyFlagsType) < 0)
                return NULL;
            if (PyType_Ready(&PyEnumType) < 0)
                return NULL;
            if (PyType_Ready(&PyTypeType) < 0)
                return NULL;
            if (PyType_Ready(&PyDebugLineType) < 0)
                return NULL;
            if (PyType_Ready(&PyNamedType) < 0)
                return NULL;
            if (PyType_Ready(&PyBindingType) < 0)
                return NULL;
            if (PyType_Ready(&PyScopeBindingType) < 0)
                return NULL;

            PyObject *m = PyModule_Create(&PyEngine_module);
            if (m == NULL)
                return NULL;

            Py_INCREF(&PyVector2Type);
            if (PyModule_AddObject(m, "Vector2", (PyObject *)&PyVector2Type) < 0) {
                Py_DECREF(&PyVector2Type);
                Py_DECREF(m);
                return NULL;
            }
            Py_INCREF(&PyVector3Type);
            if (PyModule_AddObject(m, "Vector3", (PyObject *)&PyVector3Type) < 0) {
                Py_DECREF(&PyVector3Type);
                Py_DECREF(m);
                return NULL;
            }
            Py_INCREF(&PyMatrix3Type);
            if (PyModule_AddObject(m, "Matrix3", (PyObject *)&PyMatrix3Type) < 0) {
                Py_DECREF(&PyMatrix3Type);
                Py_DECREF(m);
                return NULL;
            }
            Py_INCREF(&PyQuaternionType);
            if (PyModule_AddObject(m, "Quaternion", (PyObject *)&PyQuaternionType) < 0) {
                Py_DECREF(&PyQuaternionType);
                Py_DECREF(m);
                return NULL;
            }
            Py_INCREF(&PyDebugLineType);
            if (PyModule_AddObject(m, "DebugLine", (PyObject *)&PyDebugLineType) < 0) {
                Py_DECREF(&PyDebugLineType);
                Py_DECREF(m);
                return NULL;
            }
            Py_INCREF(&PyNamedType);
            if (PyModule_AddObject(m, "Named", (PyObject *)&PyNamedType) < 0) {
                Py_DECREF(&PyNamedType);
                Py_DECREF(m);
                return NULL;
            }

            return m;
        }

        PyMODINIT_FUNC PyInit_Environment(void)
        {

            PyObject *m = PyModule_Create(&PyEnv_module);
            if (m == NULL)
                return NULL;

            return m;
        }

        static Python3StreamRedirect sStream;
        BehaviorReceiver *sReceiver = nullptr;

        Python3Environment::Python3Environment(Root::Root &root)
            : RootComponent(root)
        {
            root.taskQueue()->addSetupSteps([this]() { return callInit(); },
                [this]() { return callFinalize(); });
        }

        Threading::Task<bool> Python3Environment::init()
        {
            PyStatus status;

#define HANDLE_STATUS()             \
    if (PyStatus_IsError(status)) { \
        LOG_ERROR(status.err_msg);  \
    }                               \
    if (PyStatus_IsExit(status)) {  \
        PyConfig_Clear(&config);    \
        co_return false;            \
    }

            PyConfig config;
            PyConfig_InitPythonConfig(&config);
            config.isolated = 1;

            std::string path = Filesystem::shippingPath() / PYTHON3_STDLIB_ZIP;

            status = PyConfig_SetBytesString(&config, &config.pythonpath_env, path.c_str());
            HANDLE_STATUS();

            status = PyConfig_SetBytesString(&config, &config.program_name, "Madgine-Python3-Env");
            HANDLE_STATUS();

            config.module_search_paths_set = 1;
            status = PyWideStringList_Append(&config.module_search_paths, config.pythonpath_env);
            HANDLE_STATUS();

            /* Add a built-in module, before Py_Initialize */
            if (PyImport_AppendInittab("Engine", PyInit_Engine) == -1) {
                LOG("Error: could not extend built-in modules table");
                mErrorCode = -1;
                co_return false;
            }

            /* Add a built-in module, before Py_Initialize */
            if (PyImport_AppendInittab("Environment", PyInit_Environment) == -1) {
                LOG("Error: could not extend built-in modules table");
                mErrorCode = -1;
                co_return false;
            }

            config.site_import = false;

            status = Py_InitializeFromConfig(&config);
            HANDLE_STATUS();
            PyConfig_Clear(&config);

            PyRun_SimpleString("import importlib");

            PyRun_SimpleString("import Environment");
            PyRun_SimpleString("import Engine");
            sStream.redirect("stdout");
            sStream.redirect("stderr");

            Python3FileLoader::getSingleton().setup();

            PyEval_SaveThread();

            co_return true;
        }

        Threading::Task<void> Python3Environment::finalize()
        {
            Python3FileLoader &loader = Python3FileLoader::getSingleton();

            for (std::pair<const std::string, Python3FileLoader::Resource> &res : loader) {
                co_await res.second.forceUnload();
            }

            lock(nullptr);

            Python3BehaviorFactory::sFactory.mBehaviorObjects.clear();
            loader.cleanup();

            sStream.reset("stdout");
            sStream.reset("stderr");

            auto result = Py_FinalizeEx();
            assert(result == 0);
        }

        std::string_view Python3Environment::key() const
        {
            return "Python3Environment";
        }

        KeyValueResult Python3Environment::execute(ValueType &retVal, std::string_view command)
        {
            Python3Lock lock;

            PyModulePtr main { "__main__" };

            PyObjectPtr result = PyRun_String(command.data(), Py_eval_input, main.getDict(), main.getDict());
            return fromPyObject(retVal, result);
        }

        PyGILState_STATE Python3Environment::lock()
        {
            // assert(PyGILState_Check() == 0);
            PyGILState_STATE handle = PyGILState_Ensure();
            assert(PyGILState_Check() == 1);
            return handle;
        }

        Log::Log *Python3Environment::unlock(PyGILState_STATE handle)
        {
            Log::Log *result = sStream.log();
            assert(PyGILState_Check() == 1);
            PyGILState_Release(handle);
            return result;
        }

        void Python3Environment::lock(BehaviorReceiver *rec)
        {
            // assert(PyGILState_Check() == 0);
            PyGILState_STATE handle = PyGILState_Ensure();
            assert(PyGILState_Check() == 1);
            assert(handle == PyGILState_UNLOCKED);
            sStream.setLog(rec ? Log::get_log(*rec) : nullptr);
            sReceiver = rec;
        }

        BehaviorReceiver *Python3Environment::unlock()
        {
            BehaviorReceiver *result = std::exchange(sReceiver, nullptr);
            sStream.setLog({});
            assert(PyGILState_Check() == 1);
            PyGILState_Release(PyGILState_UNLOCKED);
            return result;
        }

        size_t Python3Environment::totalRefCount()
        {
            PyObjectPtr refCount = PyObject_CallObject(PySys_GetObject((char *)"gettotalrefcount"), NULL);
            return PyLong_AsSsize_t(refCount);
        }

    }
}
}
