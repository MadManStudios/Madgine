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

        PyObject *sPyContinuationContextVar = nullptr;
        PyObject *sPyReceiverContextVar = nullptr;

        void handleExecutionError(BehaviorReceiver &rec, Reflect::Error error)
        {
            Python3Suspend suspend;
            rec.set_error(std::move(error));
        }

        void handleExecutionObject(BehaviorReceiver &rec, PyObject *obj)
        {
            if (!obj) {
                if (PyErr_ExceptionMatches(PyExc_EOFError)) {
                    Python3Suspend suspend;
                    rec.set_done();
                } else {
                    handleExecutionError(rec, fetchError());
                }
                return;
            }

            Python3Suspend suspend;

            if (obj == Py_None) {
                rec.set_value();
            } else if (PyUnicode_Check(obj)) {
                const char *s;
                if (!PyArg_Parse(obj, "s", &s))
                    throw 0;
                rec.set_value(std::string { s });
            } else if (PyBool_Check(obj)) {
                rec.set_value(obj == Py_True);
            } else if (PyLong_Check(obj)) {
                int i;
                if (!PyArg_Parse(obj, "i", &i))
                    throw 0;
                rec.set_value(i);
            } else if (PyDict_Check(obj)) {
                Py_INCREF(obj);
                rec.set_value(Reflect::AssociativeRange { PyDictPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
            } else if (PyList_Check(obj)) {
                Py_INCREF(obj);
                rec.set_value(Reflect::SequenceRange { PyListPtr { obj }, Engine::type_holder<VirtualRangeHelper> });
            } else if (obj->ob_type == &PyScopePtrType) {
                rec.set_value(reinterpret_cast<PyScopePtr *>(obj)->mPtr);
            } else if (PyTuple_Check(obj)) {
                size_t size = PyTuple_Size(obj);
                Reflect::ArgumentList args { std::true_type {}, size };
                for (size_t i = 0; i < args.size(); ++i) {
                    Reflect::Result result = fromPyObject(args[i], PyTuple_GetItem(obj, i));
                    if (result) {
                        rec.set_error(std::move(*result.mError));
                        return;
                    }
                }
                rec.set_value(std::move(args));
            } else {
                Py_INCREF(obj);
                rec.set_value(Reflect::ObjectPtr { std::make_unique<PyObjectInstance>(obj) });
            }
        }

        bool lock()
        {
            // assert(PyGILState_Check() == 0);
            [[maybe_unused]] PyGILState_STATE handle = PyGILState_Ensure();
            assert(PyGILState_Check() == 1);
            return handle == PyGILState_UNLOCKED;
        }

        void unlock()
        {
            assert(PyGILState_Check() == 1);
            PyGILState_Release(PyGILState_UNLOCKED);
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
            PyObjectPtr pyReceiver;
            if (PyContextVar_Get(sPyReceiverContextVar, NULL, &pyReceiver) < 0)
                return nullptr;
            BehaviorReceiver *rec = static_cast<BehaviorReceiver *>(PyCapsule_GetPointer(pyReceiver, "Receiver"));
            if (!rec)
                return nullptr;
            Debug::ContextInfo &context = Debug::get_debug_context(*rec);
            DebugLine &line = self->mLine;

            PyObjectPtr continuation;
            if (PyContextVar_Get(sPyContinuationContextVar, NULL, &continuation) < 0)
                return nullptr;
            Debug::Continuation *cont = static_cast<Debug::Continuation *>(PyCapsule_GetPointer(continuation, "Continuation"));
            if (!cont)
                return nullptr;

            if (line.mLineNr > 0 && (context.wantsPause(PyEval_GetFrame(), Debug::ContinuationType::Flow, line.mLineNr) || cont->mode() != Debug::ContinuationMode::Continue)) {
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

        void resumeCoroutine(Python3Coroutine &coro, BehaviorReceiver &rec, PyObjectPtr value)
        {
            PyObjectPtr result;
            PyContext_Enter(coro.mContext);
            if (!value) {
                PyObjectPtr exc = PyObject_CallFunction(PyExc_EOFError, NULL, NULL);
                result = PyObject_CallFunctionObjArgs(coro.mCoroutine.get("throw"), static_cast<PyObject *>(exc), NULL);
            } else if (PyExceptionInstance_Check(value)) {
                result = PyObject_CallFunctionObjArgs(coro.mCoroutine.get("throw"), static_cast<PyObject *>(value), NULL);
            } else {
                result = PyObject_Call(coro.mCoroutine.get("send"), value, NULL);
            }
            PyContext_Exit(coro.mContext);
            if (!result) {
                if (PyErr_ExceptionMatches(PyExc_StopIteration)) {
                    PyObjectPtr type, value, traceback;
                    PyErr_Fetch(&type, &value, &traceback);

                    result = value.get("value");
                }
                handleExecutionObject(rec, result);
            } else if (Py_IS_TYPE(result, &PyStateType)) {
                PyStateBase &state = reinterpret_cast<PyStateHelper *>(static_cast<PyObject *>(result))->mState;
                state.mCoroutine = &coro;
                state.mRec = &rec;
                state.resume();
            } else if (Py_IS_TYPE(result, &PyDebugLineType)) {
                Python3Suspend suspend;

                coro.mContinuation.suspend(&coro, rec, [&coro](BehaviorReceiver &rec) mutable { // TODO: Proper wrapping receiver
                    Python3Lock lock;
                    resumeCoroutine(coro, rec, toPyTuple(Reflect::ArgumentList { std::monostate {} }));
                },
                    Debug::ContinuationType::Flow);
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
                resumeCoroutine(*mCoroutine, *mRec, std::move(mResult));
            }
        }

        void PyReceiver::set_value(const Reflect::ArgumentList &values)
        {
            assert(mState.mDestruct);
            mState.mDestruct(mState);
            mState.mDestruct = nullptr;

            Python3Lock lock;
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

            Python3Lock lock;
            mState.mResult = toPyException(std::move(error));

            mState.resume();
        }

        void PyReceiver::set_done()
        {
            assert(mState.mDestruct);
            mState.mDestruct(mState);
            mState.mDestruct = nullptr;

            Python3Lock lock;
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
                return (PyObject *)self;
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