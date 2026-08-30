#include "../../python3lib.h"

#include "pyvector3.h"

#include "../pyobjectptr.h"
#include "../pyobjectutil.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        int PyVector3_init(PyVector3 *self, PyObject *args, PyObject *kwds)
        {
            float x, y, z;
            if (!PyArg_ParseTuple(args, "fff", &x, &y, &z))
                return -1;

            self->mVector = { x, y, z };
            return 0;
        }

        static PyObject *PyVector3_get(PyVector3 *self, PyObject *args)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return NULL;

            if (strlen(name) == 1) {
                switch (name[0]) {
                case 'x':
                    return PyFloat_FromDouble(self->mVector.x);
                case 'y':
                    return PyFloat_FromDouble(self->mVector.y);
                case 'z':
                    return PyFloat_FromDouble(self->mVector.z);
                }
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
            return NULL;
        }

        static int PyVector3_set(PyVector3 *self, PyObject *args, PyObject *value)
        {
            const char *name;

            if (!PyArg_Parse(args, "s", &name))
                return -1;

            if (!PyNumber_Check(value)) {
                PyErr_Format(PyExc_AttributeError, "%R only accepts numbers for it's attributes!", self);
                return -1;
            }

            PyObjectPtr floatObj = PyNumber_Float(value);
            if (!floatObj)
                return -1;

            float v = PyFloat_AsDouble(floatObj);

            if (strlen(name) == 1) {
                switch (name[0]) {
                case 'x':
                    self->mVector.x = v;
                    return 0;
                case 'y':
                    self->mVector.y = v;
                    return 0;
                case 'z':
                    self->mVector.z = v;
                    return 0;
                }
            }

            PyErr_Format(PyExc_AttributeError, "Could not find attribute '%s' in %R!", name, self);
            return -1;
        }

        PyTypeObject PyVector3Type = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                .tp_name
            = "Engine.Vector3",
            .tp_basicsize = sizeof(PyVector3),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyVector3, &PyVector3::mVector>,
            .tp_str = &PyStr<PyVector3, &PyVector3::mVector>,
            .tp_getattro = (getattrofunc)PyVector3_get,
            .tp_setattro = (setattrofunc)PyVector3_set,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of Vector3",
            .tp_init = (initproc)PyVector3_init,
            .tp_new = PyType_GenericNew
        };

    }
}
}