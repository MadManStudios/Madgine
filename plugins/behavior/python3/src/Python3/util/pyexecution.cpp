#include "../python3lib.h"

#include "pyexecution.h"

#include "Meta/reflect/objectptr.h"

#include "pydictptr.h"
#include "pylistptr.h"
#include "pyobjectinstance.h"
#include "pyobjectutil.h"
#include "pyscopeptr.h"
#include "pysender.h"
#include "python3lock.h"

///  @cond

struct _frame { }; // HACK for TypedPtr

///  @endcond

namespace Engine {
namespace Behavior {
    namespace Python3 {

        void handleExecutionError(Reflect::Error error)
        {
            Python3Suspend suspend;
            suspend.fetchReceiver()->set_error(std::move(error));
        }

        void handleExecutionObject(PyObject *obj)
        {
            if (!obj) {
                if (PyErr_ExceptionMatches(PyExc_EOFError)) {
                    Python3Suspend suspend;
                    suspend.fetchReceiver()->set_done();
                } else {
                    handleExecutionError(fetchError());
                }
                return;
            }

            Python3Suspend suspend;
            BehaviorReceiver *rec = suspend.fetchReceiver();

            if (obj == Py_None) {
                rec->set_value();
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                rec->set_value(std::string { s });
            } else if (PyBool_Check(obj)) {
                rec->set_value(obj == Py_True);
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                rec->set_value(i);
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                rec->set_value(Reflect::AssociativeRange { PyDictPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                rec->set_value(Reflect::SequenceRange { PyListPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
            } else if (obj->ob_type == &PyScopePtrType) {
                rec->set_value(reinterpret_cast<PyScopePtr *>(obj)->mPtr);
            } else if (PyTuple_Check(obj)) {
                size_t size = PyTuple_Size(obj);
                Reflect::ArgumentList args { std::true_type {}, size };
                for (size_t i = 0; i < args.size(); ++i) {
                    Reflect::Result result = fromPyObject(args[i], PyTuple_GetItem(obj, i));
                    if (result) {
                        rec->set_error(std::move(*result.mError));
                        return;
                    }
                }
                rec->set_value(std::move(args));
            } else {
                Py_INCREF(obj);
                rec->set_value(Reflect::ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
            }
        }

        static std::map<PyThreadState *, ExecutionState> sExecutionState;

        ExecutionState &executionState()
        {
            return sExecutionState.at(PyThreadState_Get());
        }

        bool lock(BehaviorReceiver *rec, Platform::Log::Log *log)
        {
            if (rec && !log)
                log = Platform::Log::get_log(*rec);
            // assert(PyGILState_Check() == 0);
            [[maybe_unused]] PyGILState_STATE handle = PyGILState_Ensure();
            assert(PyGILState_Check() == 1);
            if (handle == PyGILState_UNLOCKED) {

                auto &state = sExecutionState.try_emplace(PyThreadState_Get()).first->second;

                assert(state.mReceiver == nullptr);
                assert(state.mLog == nullptr);
                state.mLog = log;
                state.mReceiver = rec;

                LOG_DEBUG("[" << std::this_thread::get_id() << ", " << PyThreadState_Get() << "] Lock: " << rec);

                return true;
            }
            return false;
        }

        ExecutionState unlock()
        {
            auto &state = executionState();

            LOG_DEBUG("[" << std::this_thread::get_id() << ", " << PyThreadState_Get() << "] Unlock: " << state.mReceiver);

            ExecutionState result = std::exchange(state, {});
            assert(PyGILState_Check() == 1);
            PyGILState_Release(PyGILState_UNLOCKED);
            return result;
        }

        void DebugLine::stopRequested()
        {
            mContinuation.stop();
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

        PyObject *PyDebugLine_next(PyDebugLine *self)
        {
            Debug::ContextInfo &context = Debug::get_debug_context(*executionState().mReceiver);
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

        void resumeCoroutine(PyObjectPtr coro, PyObjectPtr value)
        {
            PyObjectPtr result;
            if (!value) {
                PyObjectPtr exc = PyObject_CallFunction(PyExc_EOFError, NULL, NULL);
                result = PyObject_CallFunctionObjArgs(coro.get("throw"), static_cast<PyObject *>(exc), NULL);
            } else if (PyExceptionInstance_Check(value)) {
                result = PyObject_CallFunctionObjArgs(coro.get("throw"), static_cast<PyObject *>(value), NULL);
            } else {
                result = PyObject_Call(coro.get("send"), value, NULL);
            }
            if (!result) {
                if (PyErr_ExceptionMatches(PyExc_StopIteration)) {
                    PyObjectPtr type, value, traceback;
                    PyErr_Fetch(&type, &value, &traceback);

                    result = value.get("value");
                }
                handleExecutionObject(result);
            } else if (Py_IS_TYPE(result, &PyStateType)) {
                PyStateBase &state = reinterpret_cast<PyStateHelper *>(static_cast<PyObject *>(result))->mState;
                state.mCoroutine = std::move(coro);

                state.resume();
            } else if (Py_IS_TYPE(result, &PyDebugLineType)) {
                DebugLine &debugLine = reinterpret_cast<PyDebugLine *>(static_cast<PyObject *>(result))->mLine;
                // yield(location, rec, std::forward<F>(callback), outContinuation, type, std::forward<Args>(args)...);
                PyObject *frame = coro.get("cr_frame");

                Python3Suspend suspend;
                BehaviorReceiver *rec = suspend.fetchReceiver();
                Platform::Log::Log *log = suspend.log();

                if (Execution::get_stop_token(*rec)->registerCallback(&debugLine)) {
                    debugLine.mContinuation = Debug::get_debug_context(*rec).suspend(frame, { [coro { std::move(coro) }, rec, log, ptr = &debugLine](Debug::ContinuationMode mode) mutable {
                                                                                                 Execution::get_stop_token(*rec)->unregisterCallback(ptr);
                                                                                                 Python3Lock lock { rec, log };
                                                                                                 switch (mode) {
                                                                                                 case Debug::ContinuationMode::Continue:
                                                                                                     resumeCoroutine(coro, toPyTuple(Reflect::ArgumentList { std::monostate {} }));
                                                                                                     break;
                                                                                                 case Debug::ContinuationMode::Abort:
                                                                                                     resumeCoroutine(coro, nullptr);
                                                                                                     break;
                                                                                                 }
                                                                                                 coro.reset();
                                                                                             },
                                                                                                Debug::ContinuationType::Flow });
                } else {
                    Python3Lock lock { rec, log };
                    resumeCoroutine(coro, nullptr);
                }
            } else {
                std::string typeName = PyUnicode_AsUTF8(PyType_GetName(Py_TYPE(result)));
                Python3Suspend suspend;
                throw 0;
            }
        }

        PyStateBase::~PyStateBase()
        {
            assert(!mDestruct);
            if (mDestruct)
                mDestruct(*this);
        }

        void PyStateBase::resume()
        {
            if (mFlag.test_and_set()) {
                resumeCoroutine(std::move(mCoroutine), std::move(mResult));
            }
        }

        void PyReceiver::set_value(const Reflect::ArgumentList &values)
        {
            assert(mState.mDestruct);
            mState.mDestruct(mState);
            mState.mDestruct = nullptr;

            Python3Lock lock { &mReceiver };
            mState.mResult = toPyTuple(values);
            if (!mState.mResult)
                mState.mResult = PyErr_GetRaisedException();

            mState.resume();
        }

        void PyReceiver::set_error(Reflect::Error error)
        {
            assert(mState.mDestruct);
            mState.mDestruct(mState);
            mState.mDestruct = nullptr;

            Python3Lock lock { &mReceiver };
            mState.mResult = toPyException(std::move(error));

            mState.resume();
        }

        void PyReceiver::set_done()
        {
            assert(mState.mDestruct);
            mState.mDestruct(mState);
            mState.mDestruct = nullptr;

            Python3Lock lock { &mReceiver };
            mState.mResult.reset();

            mState.resume();
        }

        PyObject *PyState_send(PyStateHelper *self,
            PyObject *const *args,
            Py_ssize_t nargs)
        {
            [[maybe_unused]] PyStateBase &state = self->mState;
            assert(state.mFlag.test());

            if (PyExceptionInstance_Check(args[0])) {
                Py_IncRef(args[0]);
                PyErr_SetRaisedException(args[0]);
            } else {
                PyObjectPtr emptyTuple = PyTuple_New(0);
                PyObject *exc = PyObject_Call(PyExc_StopIteration, emptyTuple, NULL);
                PyObject_SetAttrString(exc, "value", args[0]);

                PyErr_SetObject(PyExc_StopIteration, exc);
            }
            return nullptr;
        }

        PyObject *PyState_next(PyStateHelper *self)
        {
            PyStateBase &state = self->mState;
            if (state.mFlag.test()) {
                PyObject *args = state.mResult;
                return PyState_send(self, &args, 1);
            } else {
                Py_INCREF(self);                
                return (PyObject*)self;
            }
        }

        PyMethodDef PyStateMethods[] = {
            { "send", (PyCFunction)PyState_send, METH_FASTCALL, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        PyObject *PyState_Alloc(size_t size)
        {
            void *mem = PyObject_Malloc(size);

            return PyObject_Init(reinterpret_cast<PyObject *>(mem), &PyStateType);
        }

        void PyState_Dealloc(PyStateHelper *state)
        {
            state->mState.~PyStateBase();
            PyObject_Free(state);
        }

        PyTypeObject PyStateType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.ExecutionState",
            .tp_basicsize = sizeof(PyStateBase),
            .tp_itemsize = 0,
            .tp_dealloc = (destructor)PyState_Dealloc,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "helper for Execution states",
            .tp_iternext = (iternextfunc)PyState_next,
            .tp_methods = PyStateMethods,
            .tp_new = PyType_GenericNew,
        };

    }
}
}