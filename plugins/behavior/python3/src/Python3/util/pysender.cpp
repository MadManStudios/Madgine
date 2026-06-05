#include "../python3lib.h"

#include "pysender.h"

#include "Madgine/behavior/behaviorreceiver.h"

#include "pyobjectptr.h"
#include "pyobjectutil.h"
#include "python3lock.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        void SenderReceiver::set_value(const Reflect::ArgumentList &values)
        {
            SenderState &state = mState;
            destruct(state.mInnerState);

            Python3Lock lock { &state.mReceiver };
            state.mResult = toPyTuple(values);

            state.resume();
        }

        void SenderReceiver::set_error(Reflect::Error error)
        {
            SenderState &state = mState;
            destruct(state.mInnerState);

            Python3Lock lock { &state.mReceiver };
            throw "TODO";
            //state.mResult = fromKeyValueError(std::move(error);

            state.resume();
        }

        void SenderReceiver::set_done()
        {
            SenderState &state = mState;
            destruct(state.mInnerState);

            Python3Lock lock { &state.mReceiver };
            state.mResult.reset();

            state.resume();
        }

        SenderState::~SenderState()
        {
        }

        void SenderState::resume()
        {
            if (mFlag.test_and_set()) {
                resumeCoroutine(std::move(mCoroutine), std::move(mResult));
            }
        }

        PyObject *PySender_await(PyObject *self)
        {
            Reflect::Sender sender = reinterpret_cast<PySender *>(self)->mSender;

            PyObject *obj = PyObject_CallObject((PyObject *)&PySenderStateType, NULL);
            Python3Unlock unlock;
            SenderState *state = &reinterpret_cast<PySenderState *>(obj)->mState;
            new (state) SenderState { *unlock.fetchReceiver() };

            construct(state->mInnerState, DelayedConstruct<SenderState::Inner> { [&]() { return Execution::connect(std::move(sender) | Execution::stoppable | Execution::with_debug_location(state->mChild), SenderReceiver { *state }); } });
            state->mInnerState->start();

            return obj;
        }

        PyObject *PySenderState_next(PyObject *self)
        {
            SenderState &state = reinterpret_cast<PySenderState *>(self)->mState;
            if (state.mFlag.test()) {
                PyObjectPtr emptyTuple = PyTuple_New(0);

                PyObject *exc = PyObject_Call(PyExc_StopIteration, emptyTuple, NULL);

                PyObject_SetAttrString(exc, "value", state.mResult);

                PyErr_SetObject(PyExc_StopIteration, exc);
                return nullptr;
            } else {
                Py_IncRef(self);
                return self;
            }
        }

        PyObject *PySenderState_send(PyObject *self,
            PyObject *const *args,
            Py_ssize_t nargs)
        {
            [[maybe_unused]] SenderState &state = reinterpret_cast<PySenderState *>(self)->mState;
            assert(state.mFlag.test());

            PyObjectPtr emptyTuple = PyTuple_New(0);

            PyObject *exc = PyObject_Call(PyExc_StopIteration, emptyTuple, NULL);

            PyObject_SetAttrString(exc, "value", args[0]);

            PyErr_SetObject(PyExc_StopIteration, exc);
            return nullptr;
        }

        PyAsyncMethods PySenderAsyncMethods = {
            .am_await = PySender_await
        };

        static PyMethodDef PySenderStateMethods[] = {
            { "send", (PyCFunction)PySenderState_send, METH_FASTCALL, "" },
            { NULL, NULL, 0, NULL } /* Sentinel */
        };

        PyTypeObject PySenderType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Sender",
            .tp_basicsize = sizeof(PySender),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PySender, &PySender::mSender>,
            .tp_as_async = &PySenderAsyncMethods,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of KeyValueSender",
            .tp_new = PyType_GenericNew,
        };

        PyTypeObject PySenderStateType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.SenderState",
            .tp_basicsize = sizeof(PySenderState),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PySenderState, &PySenderState::mState>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "state for KeyValueSender states",
            .tp_iternext = PySenderState_next,
            .tp_methods = PySenderStateMethods,
            .tp_new = PyType_GenericNew,            
        };

    }
}
}