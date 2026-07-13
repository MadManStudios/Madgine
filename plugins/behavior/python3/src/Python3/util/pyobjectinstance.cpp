#include "../python3lib.h"

#include "pyobjectinstance.h"

#include "Meta/reflect/value.h"

#include "pyobjectiter.h"
#include "pyobjectutil.h"
#include "python3lock.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        PyObjectInstance::PyObjectInstance(PyObject *obj)
            : mPtr(obj)
        {
        }

        PyObjectInstance::~PyObjectInstance()
        {
            Python3InnerLock lock;
            mPtr.reset();
        }

        Reflect::Result PyObjectInstance::getValue(Reflect::Value &retVal, std::string_view name) const
        {
            return fromPyObject(retVal, mPtr.get(name));
        }

        void PyObjectInstance::setValue(std::string_view name, const Reflect::Value &value)
        {
            throw 0;
        }

        std::map<std::string_view, Reflect::Value> PyObjectInstance::values() const
        {
            Python3InnerLock lock;
            std::map<std::string_view, Reflect::Value> results;
            std::ranges::transform(mPtr, std::inserter(results, results.end()), [](std::pair<PyObject *, PyObject *> p) {
                Reflect::Value v;
                fromPyObject(v, p.second);
                return std::make_pair(PyUnicode_AsUTF8(p.first), std::move(v));
            });
            return results;
        }

        Reflect::Result PyObjectInstance::call(Reflect::Value &retVal, const Reflect::ArgumentList &args)
        {
            Python3InnerLock lock;

            return fromPyObject(retVal, mPtr.call(args));
        }

        std::string PyObjectInstance::descriptor() const
        {
            Python3InnerLock lock;
            PyObjectPtr repr = PyObject_Repr(mPtr);
            return PyUnicode_AsUTF8(repr);
        }

        PyObject *PyObjectInstance::get() const
        {
            return mPtr;
        }

    }
}
}