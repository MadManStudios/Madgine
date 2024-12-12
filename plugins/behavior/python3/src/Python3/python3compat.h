
typedef struct _frame PyFrameObject;



#if PY_MINOR_VERSION < 11
typedef PyFrameObject _PyInterpreterFrame;
#endif

int PyFrame_StackSize(PyFrameObject *frame);
PyObject *PyFrame_StackPeek(PyFrameObject *frame);
PyObject *PyFrame_StackPop(PyFrameObject *frame);
void PyFrame_StackPush(_PyInterpreterFrame *frame, PyObject *object);
MADGINE_PYTHON3_EXPORT PyCodeObject *PyFrame_GetCode2(PyFrameObject *frame);