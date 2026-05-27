#include "python3lib.h"

#include "python3behaviors.h"

#include "Meta/keyvalue/valuetype.h"
#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/named.h"
#include "Madgine/behavior/parametertuple.h"
#include "Madgine/debug/debuglocation.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "python3env.h"
#include "util/pyobjectutil.h"
#include "util/pysender.h"
#include "util/python3lock.h"

BEHAVIOR_FACTORY(Python3, Engine::Behavior::Python3::Python3BehaviorFactory)

///  @cond

struct _frame { }; // HACK for TypedPtr

///  @endcond

namespace Engine {
namespace Behavior {
    namespace Python3 {

        void DebugLine::stopRequested()
        {
            if (mContinuation) {
                mContinuation(Debug::ContinuationMode::Abort);
            }
        }

        int PyDebugLine_init(PyDebugLine *self, PyObject *args, PyObject *kwds)
        {
            new (&self->mLine) DebugLine;
            return PyArg_ParseTuple(args, "n", &self->mLine.mLineNr);
        }

        PyObject *PyDebugLine_await(PyObject *self)
        {
            Py_IncRef(self);
            return self;
        }

        extern BehaviorReceiver *sReceiver;

        PyObject *PyDebugLine_next(PyDebugLine *self)
        {
            Debug::ContextInfo &context = Debug::get_debug_context(*sReceiver);
            DebugLine &line = self->mLine;

            if (line.mLineNr > 0 && context.wantsPause(PyEval_GetFrame(), Debug::ContinuationType::Flow, line.mLineNr)) {
                line.mLineNr = 0;
                PyObject *selfObj = reinterpret_cast<PyObject *>(self);
                Py_IncRef(selfObj);
                return selfObj;
            } else {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }
        }

        PyAsyncMethods PyDebugLineAsyncMethods = {
            .am_await = PyDebugLine_await
        };

        PyTypeObject PyDebugLineType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.DebugLine",
            .tp_basicsize = sizeof(PyDebugLine),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyDebugLine, &PyDebugLine::mLine>,
            .tp_as_async = &PyDebugLineAsyncMethods,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python helper for debugging",            
            .tp_iternext = (iternextfunc)PyDebugLine_next,                
            .tp_init = (initproc)PyDebugLine_init,
            .tp_new = PyType_GenericNew,        
        };

        PyObject *PyNamed_resolve(PyObject *cls, PyObject *nameObj)
        {
            if (!PyUnicode_Check(nameObj)) {
                PyErr_SetString(PyExc_TypeError, "Name must be a string");
                return nullptr;
            }
            const char *name = PyUnicode_AsUTF8(nameObj);
            ValueType v;
            KeyValueResult result = get_named_d(*sReceiver, name, v);
            if (result) {
                return toPyError(std::move(*result.mError));
            }
            return toPyObject(v);
        }

        static PyMethodDef PyNamedMethods[] = {
            { "__class_getitem__", Py_GenericAlias, METH_O | METH_CLASS, "" },
            { "resolve", PyNamed_resolve, METH_O | METH_CLASS, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        PyTypeObject PyNamedType = {
            .ob_base = PyObject_HEAD_INIT(NULL)
                .tp_name
            = "Engine.Named",
            .tp_itemsize = 0,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python helper for Named annotations",
            .tp_methods = PyNamedMethods,
            .tp_new = PyType_GenericNew,
        };

        struct Python3ParameterTuple : ParameterTupleBase {

            Python3ParameterTuple(const Python3BehaviorFactory::Entry &entry)
                : mEntry(entry)
                , mValues(std::true_type {}, entry.mInfo.mArguments.size())
            {
                for (size_t i = 0; i < mValues.size(); ++i) {
                    ExtendedValueTypeDesc type = entry.mInfo.mArguments[i].mType;
                    if (type.mType == ExtendedValueTypeEnum::VariantType) {
                        mValues[i].setType(type.unwrapVariant().first);
                    } else {
                        mValues[i].setType(type);
                    }
                }
            }

            size_t size() const override
            {
                return mValues.size();
            }

            std::string_view name(size_t index) const override
            {
                return mEntry.mInfo.mArguments[index].mName;
            }

            ExtendedValueTypeDesc type(size_t index) const override
            {
                return mEntry.mInfo.mArguments[index].mType;
            }

            std::unique_ptr<ParameterTupleBase> clone() override
            {
                return std::make_unique<Python3ParameterTuple>(*this);
            }

            ScopePtr customScopePtr() override
            {
                return { this, &mEntry.mMetaTable };
            }

            Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in) override
            {
                for (size_t i = 0; i < mValues.size(); ++i) {
                    STREAM_PROPAGATE_ERROR(Serialize::read(in, mValues[i], mEntry.mInfo.mArguments[i].mName.data()));
                }
                return {};
            }

            void write(Serialize::CallerHierarchyFormattedSerializeStream out) override
            {
                for (size_t i = 0; i < mValues.size(); ++i) {
                    Serialize::write(out, mValues[i], mEntry.mInfo.mArguments[i].mName.data());
                }
            }

            const Python3BehaviorFactory::Entry &mEntry;
            ArgumentList mValues;
        };

        std::unique_ptr<Accessor[]> Python3BehaviorFactory::Entry::accessors(const PythonFunctionInfo &info)
        {
            std::unique_ptr<Accessor[]> accessors = std::make_unique<Accessor[]>(info.mArguments.size() + 1);

            for (size_t i = 0; i < info.mArguments.size(); ++i) {
                accessors[i] = Accessor {
                    info.mArguments[i].mName.data(),
                    nullptr,
                    [](const Accessor *self, ValueType &out, const ValueType &scope) -> KeyValueResult {
                        size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                        return ValueType_unwrap(out, [index](Python3ParameterTuple &tuple) { return tuple.mValues[index]; }, scope);
                    },
                    [](const Accessor *self, const ValueType &scope, const ValueType &val) -> KeyValueResult {
                        size_t index = self - (*scope.type().mSecondary.mMetaTable)->mMembers;
                        return ValueType_unwrap([&](Python3ParameterTuple &tuple) { tuple.mValues[index] = val; }, scope);
                    },
                    info.mArguments[i].mType,
                    info.mArguments[i].mFlags
                };
            }

            return accessors;
        }

        Python3BehaviorFactory::Entry::Entry(PyObjectPtr function)
            : mFunction(std::move(function))
            , mInfo(Python3FileLoader::functionInfo(mFunction))
            , mTupleName(mInfo.mName + "Parameters")
            , mTupleAccessors(accessors(mInfo))
            , mMetaTable(&mMetaTablePtr, mTupleName.c_str(), mTupleAccessors.get())
        {
            mMetaTable.mBase = &table<Python3ParameterTuple>;
        }

        struct Python3BehaviorState : BehaviorReceiver {
            Python3BehaviorState(PyObjectPtr function, const ArgumentList &args)
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
                    handleKeyValueError(fromPyError(mCoroutine));
                    return;
                }

                resumeCoroutine(mCoroutine, toPyTuple(ArgumentList { std::monostate {} }));
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

            using result_type = KeyValueError;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<ArgumentList>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, Python3BehaviorSender &&sender, Rec &&rec)
            {
                return VirtualBehaviorState<Rec, Python3BehaviorState> { std::forward<Rec>(rec), std::move(sender.mFunction), std::move(sender.mArguments) };
            }

            PyObjectPtr mFunction;
            ArgumentList mArguments;
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

            Python3BehaviorFactory::sFactory.mBehaviorObjects.try_emplace(name, patchedFn);

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
            ptr.setupAs<std::pair<const std::string_view, Entry> *>() = &*sFactory.mBehaviorObjects.find(name);
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

        Behavior Python3BehaviorFactory::create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();
            return Python3BehaviorSender { {}, fn->second.mFunction, args.get<Python3ParameterTuple>().mValues };
        }

        ParameterTuple Python3BehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();

            return ParameterTuple { std::make_unique<Python3ParameterTuple>(fn->second) };
        }

        std::vector<ExtendedValueTypeDesc> Python3BehaviorFactory::parameterTypes(const UniqueOpaquePtr &handle) const
        {
            const std::pair<const std::string_view, Entry> *fn = handle.as<std::pair<const std::string_view, Entry> *>();
            auto types = fn->second.mInfo.mArguments | std::views::transform(&PythonFunctionArgument::mType);
            return { types.begin(), types.end() };
        }

        std::vector<ExtendedValueTypeDesc> Python3BehaviorFactory::resultTypes(const UniqueOpaquePtr &handle) const
        {
            // const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return {};
        }

        std::vector<NamedDescriptor> Python3BehaviorFactory::namedInputs(const UniqueOpaquePtr &handle) const
        {
            // const Python3FileLoader::Handle &file = handle.as<Python3FileLoader::Handle>();
            return {};
        }

        size_t Python3BehaviorFactory::subBehaviorCount(const UniqueOpaquePtr &handle) const
        {
            return 0;
        }

    }

}
}

METATABLE_BEGIN(Engine::Behavior::Python3::Python3ParameterTuple)
METATABLE_END(Engine::Behavior::Python3::Python3ParameterTuple);