#include "../python3lib.h"

#include "pyobjectiter.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        
        PyObjectIterator::PyObjectIterator(PyObjectPtr object)
            : mObject(std::move(object))
        {
        }

        PyObjectIterator::PyObjectIterator(PyObjectPtr object, PyListPtr::iterator it)
            : mObject(std::move(object))
            , mIt(std::move(it))            
        {
        }

        PyObjectIterator &PyObjectIterator::operator++()
        {
            ++mIt;
            return *this;
        }

        void PyObjectIterator::operator++(int)
        {
            ++(*this);
        }

        std::pair<PyObject *, PyObjectPtr> PyObjectIterator::operator*() const
        {
            PyObject *key = *mIt;
            return { key, PyObject_GetAttr(mObject, key) };
        }

        bool PyObjectIterator::operator!=(const PyObjectIterator &other) const
        {
            assert(mObject == other.mObject);
            return mIt != other.mIt;
        }

        bool PyObjectIterator::operator==(const PyObjectIterator &other) const
        {
            assert(mObject == other.mObject);
            return mIt == other.mIt;
        }

    }
}
}