#include "../python3lib.h"

#include "pyexecution.h"

#include "python3lock.h"

#include "../python3debugger.h"

#if PY_MINOR_VERSION < 11
#    include <frameobject.h>
#else
#    include <internal/pycore_frame.h>
#endif

#include "pydictptr.h"
#include "pyobjectutil.h"

#include "Meta/keyvalue/argumentlist.h"

#include "../python3env.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        static bool sUnwindable = false;

        extern bool sSuspending;

        struct PySuspendException {
            PyObject **mStacktop;
            int mBlock;
            Closure<void(BehaviorReceiver &, std::vector<PyFramePtr>, Log::Log *, std::stop_token)> mCallback;
            std::vector<PyFramePtr> mFrames;
        };

        struct PySuspendExceptionObject {
            PyException_HEAD
                PySuspendException mException;
        };

        PyTypeObject PySuspendExceptionType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "PySuspendException",
            .tp_basicsize = sizeof(PySuspendExceptionObject),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PySuspendExceptionObject, &PySuspendExceptionObject::mException>,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASE_EXC_SUBCLASS,
            .tp_doc = "Utility exception for Python suspension",
            .tp_base = (PyTypeObject *)PyExc_Exception,
            .tp_new = PyType_GenericNew,
        };

        struct PyBehaviorScope {
            BehaviorReceiver *mReceiver;
            PyObjectPtr mInner;
        };

        struct PyBehaviorScopeObject {
            PyObject_HEAD
                PyBehaviorScope mScope;
        };

        static PyObject *
        PyBehaviorScope_subscript(PyBehaviorScopeObject *self, PyObject *key)
        {

            PyObject *var = PyObject_GetItem(self->mScope.mInner, key);
            if (var) {
                return var;
            }

            const char *name;

            if (!PyArg_Parse(key, "s", &name))
                return NULL;

            ValueType v;
            BehaviorError error = self->mScope.mReceiver->getBinding(name, v);
            if (error.mResult == BehaviorResult { BehaviorResult::SUCCESS }) {
                return toPyObject(v);
            } else {
                throw 0;
                return nullptr;
            }
        }

        static int
        PyBehaviorScope_assign_subscript(PyBehaviorScopeObject *self, PyObject *key, PyObject *value)
        {
            return PyObject_SetItem(self->mScope.mInner, key, value);
        }

        /* static PyObject *OwnedScopePtr_iter(const ScopePtr &p)
        {
            if (!p) {
                PyErr_SetString(PyExc_TypeError, "Nullptr is not iterable!");
                return NULL;
            }
            ScopeIterator proxyIt = p.find("__proxy");
            if (proxyIt != p.end()) {
                ValueType proxy;
                proxyIt->value(proxy);
                if (proxy.is<ScopePtr>()) {
                    return OwnedScopePtr_iter(proxy.as<ScopePtr>());
                }
            }
            return toPyObject(p.begin());
        }

        static PyObject *
        PyOwnedScopePtr_iter(PyBehaviorScopeObject *self)
        {
            return OwnedScopePtr_iter(self->mPtr.get());
        }*/

        static PyMappingMethods PyBehaviorScopeMethods = {
            .mp_subscript = (binaryfunc)&PyBehaviorScope_subscript,
            .mp_ass_subscript = (objobjargproc)&PyBehaviorScope_assign_subscript,
        };

        PyTypeObject PyBehaviorScopeType = {
            .ob_base = PyObject_HEAD_INIT(NULL).tp_name = "Engine.BehaviorScope",
            .tp_basicsize = sizeof(PyBehaviorScopeObject),
            .tp_dealloc = &PyDealloc<PyBehaviorScopeObject, &PyBehaviorScopeObject::mScope>,
            //.tp_getattro = (getattrofunc)PyBehaviorScope_get,
            .tp_as_mapping = &PyBehaviorScopeMethods,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Scope wrapper to access Behavior scope variables",
            //.tp_iter = (getiterfunc)PyOwnedScopePtr_iter,
            .tp_new = PyType_GenericNew,
        };

        PyObject *evalFrame(
#if PY_MINOR_VERSION >= 9
            PyThreadState *tstate,
#endif
            _PyInterpreterFrame *frame, int throwExc)
        {
            PyObject *result = _PyEval_EvalFrameDefault(
#if PY_MINOR_VERSION >= 9
                tstate,
#endif
                frame, throwExc);
            if (!result && PyErr_ExceptionMatches((PyObject *)&PySuspendExceptionType)) {
                PyObjectPtr type, value, traceback;
                PyErr_Fetch(&type, &value, &traceback);

                PySuspendException &suspend = reinterpret_cast<PySuspendExceptionObject *>(static_cast<PyObject *>(value))->mException;
                // frame-> = suspend.mStacktop;
                // frame-> f_iblock = suspend.mBlock;
                suspend.mStacktop = nullptr;
                suspend.mBlock = 0;

                suspend.mFrames.push_back(PyFramePtr::fromBorrowed(frame));

                return value.release();
            }
            return result;
        }

        static void handleResult(PyObject *result, BehaviorReceiver &receiver, std::vector<PyFramePtr> *frames = nullptr)
        {
            if (result && result->ob_type == &PySuspendExceptionType) {
                PySuspendException &suspend = reinterpret_cast<PySuspendExceptionObject *>(result)->mException;

                assert(suspend.mStacktop == nullptr);
                assert(suspend.mBlock == 0);

                Closure<void(BehaviorReceiver &, std::vector<PyFramePtr>, Log::Log *, std::stop_token)> callback = std::move(suspend.mCallback);
                std::vector<PyFramePtr> suspendedFrames = std::move(suspend.mFrames);

                Py_DECREF(result);

                sSuspending = false;

                Python3Unlock unlock;

                if (frames) {
                    std::ranges::move(*frames, std::back_inserter(suspendedFrames));
                    frames->clear();
                }

                callback(receiver, std::move(suspendedFrames), unlock.log(), unlock.st());

            } else {
                if (!frames || frames->empty()) {
                    fromPyObject(receiver, result);
                } else {
                    PyFrame_StackPush(frames->front(), result);
                }
            }
        }

        void evalCode(BehaviorReceiver &receiver, CodeObject code)
        {
            assert(sUnwindable == false);
            sUnwindable = true;
            PyObject *result = PyEval_EvalCode(code.mCode.release(), code.mGlobals.release(), code.mLocals.release());
            handleResult(result, receiver);
            sUnwindable = false;
        }

        void evalFrame(BehaviorReceiver &receiver, PyFramePtr frame)
        {
            std::vector<PyFramePtr> frames;
            frames.push_back(std::move(frame));
            evalFrames(receiver, std::move(frames));
        }

        void evalFrames(BehaviorReceiver &receiver, std::vector<PyFramePtr> frames)
        {
            assert(sUnwindable == false);
            sUnwindable = true;
            while (!frames.empty()) {
                auto it = frames.begin();
                PyFramePtr frame = std::move(*it);
                frames.erase(it);
#if PY_MINOR_VERSION < 11
                PyObject *result = PyEval_EvalFrame(frame);
#else
                PyFrameObject _frame;
                _frame.f_frame = frame;
                PyObject *result = PyEval_EvalFrame(&_frame);
#endif
                handleResult(result, receiver, &frames);
            }
            sUnwindable = false;
        }

        int executionTrace(PyObject *obj, PyFrameObject *frame, int event, PyObject *arg)
        {

            if (event == PyTrace_OPCODE) {
                if (PyFrame_StackSize(frame) > 0 && PyFrame_StackPeek(frame)->ob_type == &PySuspendExceptionType) {
                    PyObject *suspendEx = PyFrame_StackPop(frame);

                    PySuspendException &suspend = reinterpret_cast<PySuspendExceptionObject *>(suspendEx)->mException;

                    assert(suspend.mStacktop == nullptr);
                    assert(suspend.mBlock == 0);

                    //--frame->f_frame->prev_instr;
                    // suspend.mStacktop = frame->f_stacktop;
                    // frame->f_stacktop = frame->f_valuestack;
                    // suspend.mBlock = frame->f_iblock;
                    // frame->f_iblock = 0;

                    PyErr_SetObject((PyObject *)&PySuspendExceptionType, suspendEx);
                    Py_DECREF(suspendEx);
                    return -1;
                }
            }
            return 0;
        }

        bool stackUnwindable()
        {
            return sUnwindable;
        }

        PyObject *suspend(Closure<void(BehaviorReceiver &, std::vector<PyFramePtr>, Log::Log *, std::stop_token)> callback)
        {
            assert(stackUnwindable());

            PyObject *suspendEx = PyObject_CallObject((PyObject *)&PySuspendExceptionType, nullptr);
            PySuspendException &suspend = reinterpret_cast<PySuspendExceptionObject *>(static_cast<PyObject *>(suspendEx))->mException;
            new (&suspend) PySuspendException;

            suspend.mCallback = std::move(callback);

            return suspendEx;
        }

        BehaviorError fetchError()
        {
            PyObjectPtr type, value, traceback;
            PyErr_Fetch(&type, &value, &traceback);

            const char *filename = "";
            size_t line = 0;

            if (traceback) {
                PyTracebackObject *tb = reinterpret_cast<PyTracebackObject *>(static_cast<PyObject *>(traceback));
                while (tb->tb_next)
                    tb = tb->tb_next;

                filename = PyUnicode_AsUTF8(PyFrame_GetCode2(tb->tb_frame)->co_filename);
                line = PyFrame_GetCode2(tb->tb_frame)->co_firstlineno;
            }

            PyObjectPtr str = PyObject_Str(value);
            const char *errorMessage = PyUnicode_AsUTF8(str);

            std::string msg;
            if (errorMessage)
                msg = errorMessage;

            return { BehaviorResult::UNKNOWN_ERROR, msg, filename, line };
        }

        void setupExecution()
        {
#if PY_MINOR_VERSION < 9
            PyGILState_GetThisThreadState()->interp->eval_frame = &evalFrame;
#else
            PyRun_SimpleString("import inspect");
            PyRun_SimpleString("inspect.currentframe().f_trace_opcodes = True");

            _PyInterpreterState_SetEvalFrameFunc(PyInterpreterState_Get(), &evalFrame);
#endif

            PyEval_SetTrace(&executionTrace, nullptr);
        }

        ExecutionState::ExecutionState(ExecutionData data)
            : mData(std::move(data))
        {
        }

        ExecutionState::~ExecutionState()
        {
            std::visit(overloaded {
                           [](CodeObject &code) {
                               if (code.mCode || code.mGlobals || code.mLocals) {
                                   Python3Lock lock;
                                   code.mCode.reset();
                                   code.mGlobals.reset();
                                   code.mLocals.reset();
                               }
                           },
                           [](PyFramePtr &frame) {
                               if (frame) {
                                   Python3Lock lock;
                                   frame.reset();
                               }
                           },
                           [](BehaviorError &error) {} },
                mData);
        }

        void ExecutionState::start()
        {
            Python3Lock lock { log(), stopToken() };

            Python3Debugger::Guard guard { debugLocation() };

            std::visit(overloaded {
                           [this](CodeObject code) {
                               PyObject *wrapped = PyObject_CallObject((PyObject *)&PyBehaviorScopeType, NULL);
                               new (&reinterpret_cast<PyBehaviorScopeObject *>(wrapped)->mScope) PyBehaviorScope { this, code.mLocals };
                               code.mLocals = wrapped;

                               evalCode(*this, std::move(code));
                           },
                           [this](PyFramePtr frame) {
                               PyObject *wrapped = PyObject_CallObject((PyObject *)&PyBehaviorScopeType, NULL);
                               new (&reinterpret_cast<PyBehaviorScopeObject *>(wrapped)->mScope) PyBehaviorScope { this, frame->f_locals };
                               frame->f_locals = wrapped;

                               evalFrame(*this, std::move(frame));
                           },
                           [this](BehaviorError error) {
                               set_error(std::move(error));
                           } },
                std::move(mData));
        }

        void tag_invoke(Execution::visit_state_t, ExecutionSender &sender, CallableView<void(const Execution::StateDescriptor &)> visitor)
        {
            visitor(Execution::State::SubLocation {});
        }
    }
}
}