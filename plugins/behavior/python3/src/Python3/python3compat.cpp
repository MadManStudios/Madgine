#include "python3lib.h"

#if PY_MINOR_VERSION < 11
#    include <frameobject.h>
#else
#    define Py_BUILD_CORE
#    include <internal/pycore_frame.h>
#endif

int PyFrame_StackSize(PyFrameObject *frame)
{
#if PY_MINOR_VERSION < 11
    return frame->f_stacktop - frame->f_valuestack;
#else
    return frame->f_frame->stacktop;
#endif
}

PyObject *PyFrame_StackPeek(PyFrameObject *frame)
{
#if PY_MINOR_VERSION < 11
    return frame->f_stacktop[-1];
#else
    return _PyFrame_StackPeek(frame->f_frame);
#endif
}

PyObject *PyFrame_StackPop(PyFrameObject *frame)
{
#if PY_MINOR_VERSION < 11
    PyObject *value = frame->f_stacktop[-1];
    --frame->f_stacktop;
    return value;
#else
    return _PyFrame_StackPop(frame->f_frame);
#endif
}

void PyFrame_StackPush(_PyInterpreterFrame *frame, PyObject *object)
{
#if PY_MINOR_VERSION < 11
    *frame->f_stacktop++ = object;
#else
    _PyFrame_StackPush(frame, object);
#endif
}

PyCodeObject *PyFrame_GetCode2(PyFrameObject *frame)
{
#if PY_MINOR_VERSION < 11
    return frame->f_code;
#else
    return PyFrame_GetCode(frame);
#endif
}
