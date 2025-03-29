#pragma once

#include "pyobjectptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {
    
        struct MADGINE_PYTHON3_EXPORT PyListPtr : PyObjectPtr {
            PyListPtr() = default;
            PyListPtr(PyObject *list);
            PyListPtr(const std::vector<PyObjectPtr> &values);

            using PyObjectPtr::operator=;

            static PyListPtr fromBorrowed(PyObject *object);

            using value_type = PyObject*;

            struct MADGINE_PYTHON3_EXPORT iterator {
                using value_type = PyObject*;
                using difference_type = ptrdiff_t;

                iterator() = default;
                iterator(PyObjectPtr list);

                PyObject *operator*() const;
                iterator &operator++();
                iterator operator++(int);
                bool operator==(const iterator &) const;

            private:
                PyObjectPtr mList;
                Py_ssize_t mIndex = 0;
            };

            iterator begin() const;
            iterator end() const;

            size_t size() const;
        };
    
    }
}
}