#include "python3lib.h"

#include "python3behaviors.h"

#include "Meta/reflect/value.h"
#include "Meta/type/storageops.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/behaviordescriptor.h"
#include "Madgine/behavior/context.h"
#include "Madgine/behavior/dynamicparametertuple.h"
#include "Madgine/behavior/parametertuple.h"
#include "Madgine/debug/debuglocation.h"

#include "Meta/reflect/metatable_impl.h"

#include "python3env.h"
#include "util/pyobjectutil.h"
#include "util/pysender.h"
#include "util/python3lock.h"
#include "util/pytype.h"

BEHAVIOR_FACTORY(Python3, Engine::Behavior::Python3::Python3BehaviorFactory)

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObject *PyContextual_resolve(PyObject *cls, PyObject *typeObj)
        {

            const Type::TypeName *type = nullptr;

            if (PyObject_TypeCheck(typeObj, &PyTypeType)) {
                type = ((PyType *)typeObj)->mType;
            } else if (PyModule_Check(typeObj)) {
                type = Type::resolveTypeName(PyModule_GetName(typeObj), ".");
            }

            if (!type) {
                PyErr_SetString(PyExc_TypeError, "type must be a TypeName");
                return nullptr;
            }            

            Reflect::Value v;
            Reflect::Result result = Reflect::get_reflect_contextual(*executionState().mReceiver, v, type->mMetaTable);
            if (result) {
                return toPyError(std::move(*result.mError));
            }
            return toPyObject(v);
        }

        static PyMethodDef PyContextualMethods[] = {
            { "__class_getitem__", Py_GenericAlias, METH_O | METH_CLASS, "" },
            { "resolve", PyContextual_resolve, METH_O | METH_CLASS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        PyTypeObject PyContextualType = {
            .ob_base = PyObject_HEAD_INIT(NULL)
                .tp_name
            = "Engine.Contextual",
            .tp_itemsize = 0,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python helper for Named annotations",
            .tp_methods = PyContextualMethods,
            .tp_new = PyType_GenericNew,
        };

        Python3BehaviorFactory::Entry::Entry(const char *name, PyObjectPtr function, Reflect::ExtendedType type, std::list<std::string> nameCache, std::vector<BehaviorDescriptor::Parameter> parameters)
            : mFunction(std::move(function))
            , mNameCache(std::move(nameCache))
            , mReturnType(Reflect::ExtendedTypeEnum::GenericType)
            , mMetaTable(&mMetaTablePtr, mTupleName.c_str(), mTupleAccessors.get(), nullptr)
            , mTupleName(name + "Parameters"s)
            , mParameters(std::move(parameters))
            , mDescriptor { mParameters, { &mReturnType, 1 }, 0 }
            , mTupleAccessors(parameterAccessors(mParameters))
        {
        }

        struct Python3BehaviorState : BehaviorReceiver {
            Python3BehaviorState(PyObjectPtr function, const Reflect::ArgumentList &args)
            {
                Python3Lock lock;
                mCoroutine = function.call(args);
                if (!mCoroutine) {
                    mCoroutine = PyErr_GetRaisedException();
                }
            }

            ~Python3BehaviorState()
            {
                Python3Lock lock;
                mCoroutine.reset();
            }

            void start()
            {
                Python3Lock lock { this };
                if (PyExceptionInstance_Check(mCoroutine)) {
                    handleExecutionError(fromPyError(mCoroutine));
                    return;
                }

                resumeCoroutine(mCoroutine, toPyTuple(Reflect::ArgumentList { std::monostate {} }));
            }

            void stop()
            {
            }

            friend auto tag_invoke(Execution::visit_state_t, Python3BehaviorState *state, auto &&visitor)
            {
                visitor(Execution::State::DebugLocation { state ? static_cast<PyObject *>(state->mCoroutine) : nullptr });
            }

            PyObjectPtr mCoroutine;
        };

        struct Python3BehaviorSender : Execution::base_sender {

            using result_type = Reflect::Error;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<Reflect::ArgumentList>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, Python3BehaviorSender &&sender, Rec &&rec)
            {
                return VirtualBehaviorState<Rec, Python3BehaviorState> { std::forward<Rec>(rec), std::move(sender.mFunction), std::move(sender.mArguments) };
            }

            PyObjectPtr mFunction;
            Reflect::ArgumentList mArguments;
        };

        PyObject *PyEngine_Behavior_decorator(PyObject *self,
            PyObject *const *args,
            Py_ssize_t nargs)
        {
            if (nargs != 1)
                throw 0;
            PyObject *fn = args[0];
            if (!PyFunction_Check(fn))
                throw 0;

            PyObject *code = PyFunction_GetCode(fn);
            PyObject *flagsObject = PyObject_GetAttrString(code, "co_flags");

            if (!PyNumber_Check(flagsObject))
                throw 0;

            long flags = PyLong_AsLong(flagsObject);
            if (flags == -1)
                throw 0;

            if (!(flags & CO_COROUTINE)) {
                PyErr_SetString(PyExc_AssertionError, "Only async functions may be Behaviors");
                return nullptr;
            }

            PyObject *nameObject = PyObject_GetAttrString(fn, "__name__");

            if (!PyUnicode_Check(nameObject))
                throw 0;

            const char *name = PyUnicode_AsUTF8(nameObject);

            PyModulePtr instrument { "instrument" };
            if (!instrument)
                return nullptr;
            PyObjectPtr patchedFn = instrument.get("patchFunction").call("(O)", fn);
            if (!patchedFn)
                return nullptr;

            PyObjectPtr signature = PyModulePtr { "inspect" }.get("signature").call("(O)", (PyObject *)patchedFn);

            Reflect::ExtendedType returnType = PyToValueTypeDesc(signature.get("return_annotation"));

            PyObjectPtr parameters = signature.get("parameters");

            PyObjectPtr iter = PyObject_GetIter(parameters);

            std::list<std::string> nameCache;
            std::vector<BehaviorDescriptor::Parameter> parameterList;
            while (PyObjectPtr key = PyIter_Next(iter)) {
                PyObjectPtr parameter = PyObject_GetItem(parameters, key);
                PyObjectPtr type = parameter.get("annotation");

                PyObjectPtr ascii = PyUnicode_AsASCIIString(key);
                std::string &name = nameCache.emplace_back(PyBytes_AsString(ascii));

                if (Py_IS_TYPE(type, &Py_GenericAliasType)) {
                    type = PyTuple_GetItem(type.get("__args__"), 0);
                    const Type::StorageOps *ops = PyToStorageOps(type);
                    if (!ops) {
                        /* PyErr_Format(PyExc_AssertionError, "Parameter of type %R does not have storage defined!", (PyObject *)type);
                        return nullptr;*/
                        parameterList.push_back({ name, &storageOps<std::monostate> });
                    } else {
                        parameterList.push_back({ name, Type::resolveVariantStorageOps({ storageOps<std::monostate>, ops }).mSelf });
                    }
                } else {
                    const Type::StorageOps *ops = PyToStorageOps(type);
                    if (!ops) {
                        PyErr_Format(PyExc_AssertionError, "Parameter of type %R does not have storage defined!", (PyObject *)type);
                        return nullptr;
                    }

                    parameterList.push_back({ name, ops->mSelf });
                }
            }

            Python3BehaviorFactory::sFactory.mBehaviorObjects.try_emplace(name, name, patchedFn, returnType, std::move(nameCache), std::move(parameterList));

            Py_IncRef(fn);
            return fn;
        }

        std::vector<std::string_view> Python3BehaviorFactory::names() const
        {
            const auto &names = sFactory.mBehaviorObjects | std::ranges::views::transform(&std::pair<const std::string_view, Entry>::first);
            return { names.begin(), names.end() };
        }

        UniqueOpaquePtr Python3BehaviorFactory::load(std::string_view name) const
        {
            UniqueOpaquePtr ptr;
            auto it = sFactory.mBehaviorObjects.find(name);
            if (it != sFactory.mBehaviorObjects.end())
                ptr.setupAs<std::pair<const std::string_view, Entry> *>() = &*it;
            return ptr;
        }

        Threading::TaskFuture<bool> Python3BehaviorFactory::state(const UniqueOpaquePtr &handle) const
        {
            return true;
        }

        void Python3BehaviorFactory::release(UniqueOpaquePtr &ptr) const
        {
            ptr.release<std::pair<const std::string_view, Entry> *>();
        }

        std::string_view Python3BehaviorFactory::name(const UniqueOpaquePtr &handle) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();

            return fn->first;
        }

        Behavior Python3BehaviorFactory::create(const UniqueOpaquePtr &handle, const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();
            return Python3BehaviorSender { {}, fn->second.mFunction, args };
        }

        ParameterTuple Python3BehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();

            return ParameterTuple { fn->second.mMetaTable, fn->second.mParameters };
        }

        const BehaviorDescriptor &Python3BehaviorFactory::descriptor(const UniqueOpaquePtr &handle) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();

            return fn->second.mDescriptor;
        }
    }

}
}
