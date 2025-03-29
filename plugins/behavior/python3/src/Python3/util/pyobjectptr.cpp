#include "../python3lib.h"

#include "pyobjectptr.h"

#include "pydictptr.h"

#include "pyobjectutil.h"
#include "pyobjectiter.h"

#include "Meta/keyvalue/valuetype.h"

#include <frameobject.h>
#include <traceback.h>

#include "python3lock.h"
#include "../python3debugger.h"

#include "pymoduleptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyObjectPtr::PyObjectPtr(PyObject *object)
            : mObject(object)
        {
        }

        PyObjectPtr::PyObjectPtr(const PyObjectPtr &other)
            : mObject(other.mObject)
        {
            Py_XINCREF(mObject);
        }

        PyObjectPtr::PyObjectPtr(PyObjectPtr &&other)
            : mObject(std::exchange(other.mObject, nullptr))
        {
        }

        PyObjectPtr::~PyObjectPtr()
        {
            reset();
        }

        PyObjectPtr &PyObjectPtr::operator=(const PyObjectPtr &other)
        {
            Py_XDECREF(mObject);
            mObject = other.mObject;
            Py_XINCREF(mObject);
            return *this;
        }

        PyObjectPtr &PyObjectPtr::operator=(PyObjectPtr &&other)
        {
            std::swap(mObject, other.mObject);
            return *this;
        }

        PyObjectPtr PyObjectPtr::fromBorrowed(PyObject *object)
        {
            Py_INCREF(object);
            return object;
        }

        PyObjectPtr PyObjectPtr::get(std::string_view name) const
        {
            return PyObject_GetAttrString(mObject, name.data());
        }

        PyObjectPtr PyObjectPtr::call(const ArgumentList &args) const
        {
            PyObjectPtr tuple = PyTuple_New(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                PyTuple_SetItem(tuple, i, toPyObject(args[i]));
            }

            return call(tuple, PyObjectPtr {});
        }

        PyObjectPtr PyObjectPtr::call(const char *format, ...) const
        {
            va_list vl;
            va_start(vl, format);
            PyObjectPtr result = call(Py_VaBuildValue(format, vl), PyObjectPtr {});
            va_end(vl);
            return result;
        }

        PyObjectPtr PyObjectPtr::call(const PyDictPtr &kwargs, const char *format, ...) const
        {
            va_list vl;
            va_start(vl, format);
            PyObjectPtr result = call(Py_VaBuildValue(format, vl), kwargs);
            va_end(vl);
            return result;
        }

        PyObjectPtr PyObjectPtr::call(const PyObjectPtr &args, const PyObjectPtr &kwargs) const
        {
            return PyObject_Call(mObject, args, kwargs);
        }

        ExecutionSender PyObjectPtr::callAsync() const
        {
            return { {}, CodeObject { PyFunction_GetCode(mObject), PyFunction_GetGlobals(mObject), PyDict_New() } };
        }

        PyObjectFieldAccessor PyObjectPtr::operator[](const PyObjectPtr &name) const
        {
            return { *this, name };
        }

        void PyObjectPtr::reset()
        {
            Py_XDECREF(mObject);
            mObject = nullptr;
        }

        PyObject *PyObjectPtr::release()
        {
            return std::exchange(mObject, nullptr);
        }

        PyObjectPtr::operator bool() const
        {
            return mObject;
        }

        PyObjectPtr::operator PyObject *() const
        {
            return mObject;
        }

        PyObject **PyObjectPtr::operator&()
        {
            assert(!mObject);
            return &mObject;
        }

        PyObjectPtr PyObjectPtr::None()
        {
            Py_RETURN_NONE;
        }

        PyObjectIterator PyObjectPtr::begin() const
        {
            return { *this, PyListPtr { PyObject_Dir(mObject) }.begin() };
        }

        PyObjectIterator PyObjectPtr::end() const
        {
            return { *this };
        }

        PyObjectFieldAccessor &PyObjectFieldAccessor::operator=(const PyObjectPtr &value)
        {
            auto result = PyObject_SetItem(mObject, mKey, value);
            assert(result == 0);
            return *this;
        }

    }
}
}
