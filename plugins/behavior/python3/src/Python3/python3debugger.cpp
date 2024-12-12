#include "python3lib.h"

#include "python3debugger.h"

#include "util/pymoduleptr.h"
#include "util/pyobjectutil.h"

#include "Meta/keyvalue/boundapifunction.h"
#include "Meta/keyvalue/functiontable.h"
#include "Meta/keyvalue/typedscopeptr.h"

#include "Meta/keyvalue/valuetype.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Generic/execution/algorithm.h"

#include "util/python3lock.h"

#include "Interfaces/filesystem/path.h"

#if PY_MINOR_VERSION < 11
#    include <frameobject.h>
#else
#    define Py_BUILD_CORE
#    include <internal/pycore_frame.h>
#endif

METATABLE_BEGIN(Engine::Scripting::Python3::Python3Debugger)
METATABLE_END(Engine::Scripting::Python3::Python3Debugger)

namespace Engine {
namespace Scripting {
    namespace Python3 {

        bool sSuspending = false;
        static bool sException = false;

        extern int executionTrace(PyObject *obj, PyFrameObject *frame, int event, PyObject *arg);

        extern PyTypeObject PySuspendExceptionType;

        struct PyDebugLocationObject {
            PyObject_HEAD
                Python3DebugLocation mLocation;
            Debug::ParentLocation *mParent = nullptr;
            bool mResume = false;
            bool mSkipOnce = false;
        };

        PyTypeObject PyDebugLocationType = {
            .ob_base = PyObject_HEAD_INIT(NULL)
                           .tp_name
            = "PyDebugLocation",
            .tp_basicsize = sizeof(PyDebugLocationObject),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyDebugLocationObject, &PyDebugLocationObject::mLocation>,
            .tp_doc = "Python3 wrapper for debug location",
            .tp_new = PyType_GenericNew,
        };

        std::string Python3DebugLocation::toString() const
        {
            return file().str() + "/" + module() + ":" + std::to_string(lineNr());
        }

        std::map<std::string_view, ValueType> Python3DebugLocation::localVariables() const
        {
#if PY_MINOR_VERSION < 11
            if (!mFrame->f_locals)
                return {};
            PyDictPtr locals = PyDictPtr::fromBorrowed(mFrame->f_locals);
#else
            PyDictPtr locals = PyFrame_GetLocals(mFrame);
#endif

            std::map<std::string_view, ValueType> results;

            std::ranges::transform(locals, std::inserter(results, results.end()), [](std::pair<PyObject *, PyObject *> p) {
                return std::make_pair(PyUnicode_AsUTF8(p.first), fromPyObject(p.second));
            });

            return results;
        }

        bool Python3DebugLocation::wantsPause(Debug::ContinuationType type) const
        {
            return type == Debug::ContinuationType::Error;
        }

        Filesystem::Path Python3DebugLocation::file() const
        {
            return ""; //TODO
            Python3Lock lock;
            //return PyUnicode_AsUTF8(PyFrame_GetCode(mFrame->frame_obj)->co_filename);
        }

        std::string Python3DebugLocation::module() const
        {
            return ""; //TODO
            Python3Lock lock;
            //return PyUnicode_AsUTF8(PyFrame_GetCode(mFrame->frame_obj)->co_name);
        }

        size_t Python3DebugLocation::lineNr() const
        {
            return 0;
            Python3Lock lock;
            //return PyCode_Addr2Line(PyFrame_GetCode(mFrame->frame_obj), PyFrame_GetLasti(mFrame->frame_obj));
        }

        Python3Debugger::Guard::Guard(Debug::ParentLocation *parent)
        {
            PyObject *obj = PyObject_CallObject((PyObject *)&PyDebugLocationType, NULL);
            Python3DebugLocation *location = new (&reinterpret_cast<PyDebugLocationObject *>(obj)->mLocation) Python3DebugLocation;
            reinterpret_cast<PyDebugLocationObject *>(obj)->mParent = parent;
            PyEval_SetTrace(&Python3Debugger::trace, obj);
        }

        Python3Debugger::Guard::Guard(PyObjectPtr location)
        {
            PyEval_SetTrace(&Python3Debugger::trace, location);
        }

        Python3Debugger::Guard::~Guard()
        {
            PyEval_SetTrace(executionTrace, nullptr);
        }

        int Python3Debugger::trace(PyObject *obj, PyFrameObject *frame, int event, PyObject *arg)
        {
            assert(stackUnwindable());

            if (!sSuspending) {

                PyDebugLocationObject *location = reinterpret_cast<PyDebugLocationObject *>(obj);

                switch (event) {
                default:
                    throw 0;
                case PyTrace_CALL:
                    frame->f_trace_opcodes = true;
                    if (!location->mResume && !location->mSkipOnce) {
                        if (!location->mLocation.mFrame)
                            location->mLocation.stepInto(location->mParent);
                        location->mLocation.mFrame =
#if PY_MINOR_VERSION < 11
                            frame;
#else
                            frame;
#endif
                    }
                    break;

                case PyTrace_RETURN:
                    if (sException) {
                        sException = false;
                    } else {
                        location->mLocation.mFrame =
#if PY_MINOR_VERSION < 11
                            frame->f_back;
#else
                            frame->f_frame->previous->previous->frame_obj;
#endif
                        if (!location->mLocation.mFrame)
                            location->mLocation.stepOut(location->mParent);
                    }
                    location->mResume = frame->f_back;
                    break;

                case PyTrace_EXCEPTION:
                    if (PyTuple_GetItem(arg, 0) != (PyObject *)&PySuspendExceptionType) {
                        location->mLocation.mFrame =
#if PY_MINOR_VERSION < 11
                            frame->f_back;
#else
                            frame->f_frame->previous->previous->frame_obj;
#endif
                        if (!location->mLocation.mFrame)
                            location->mLocation.stepOut(location->mParent);
                    }
                    sException = true;
                    location->mResume = frame->f_back;
                    break;

                case PyTrace_LINE:
                    if (location->mSkipOnce) {
                        location->mSkipOnce = false;
                        break;
                    }
                    if (!location->mLocation.pass(Debug::ContinuationType::Flow)) {
                        PyObject *suspendEx = suspend([frame, location](BehaviorReceiver &receiver, std::vector<PyFramePtr> frames, Log::Log *log, std::stop_token st) {
                            _PyInterpreterFrame *frame = frames.front();
                            //++frame->prev_instr;
                            location->mLocation.yield([location, log, &receiver, st](Debug::ContinuationMode mode, std::vector<PyFramePtr> frames) mutable {
                                Python3Lock lock { log, std::move(st) };
                                Guard guard { PyObjectPtr::fromBorrowed(reinterpret_cast<PyObject *>(location)) };
                                switch (mode) {
                                case Debug::ContinuationMode::Step:
                                case Debug::ContinuationMode::Resume:
                                    location->mSkipOnce = true;
                                    {
                                        _PyInterpreterFrame *frame = frames.front();
                                        //--frame->prev_instr;
                                    }
                                    evalFrames(receiver, std::move(frames));
                                    break;
                                case Debug::ContinuationMode::Abort:
                                    frames.clear();
                                    receiver.set_done();
                                    break;
                                }
                            },
                                st, Debug::ContinuationType::Flow, std::move(frames));
                        });
                        //_PyFrame_StackPush(frame->f_frame, suspendEx);
                        PyErr_SetObject((PyObject *)&PySuspendExceptionType, suspendEx);
                        Py_DECREF(suspendEx);
                        return -1;
                    }

                    break;

                case PyTrace_OPCODE:
                    location->mResume = false;
                    break;
                }
            }

            return executionTrace(obj, frame, event, arg);
        }
    }
}
}
