#pragma once

#include "pyobjectptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT PyDictPtr : PyObjectPtr {
            PyDictPtr() = default;
            PyDictPtr(PyObject *dict);
            PyDictPtr(const std::map<std::string, PyObjectPtr> &values);

            using PyObjectPtr::operator=;

            static PyDictPtr fromBorrowed(PyObject *dict);

            using value_type = std::pair<PyObject *, PyObject *>;

            struct iterator {
                using value_type = std::pair<PyObject *, PyObject *>;
                using difference_type = ptrdiff_t;

                iterator() = default;
                iterator(PyObject *dict);

                std::pair<PyObject *, PyObject *> operator*() const;
                iterator &operator++();
                iterator operator++(int);
                bool operator==(const iterator &) const;

            private:
                PyObject *mDict = nullptr;
                Py_ssize_t mPos = 0;
                bool mDone = true;
                PyObject *mKey = nullptr;
                PyObject *mValue = nullptr;
            };

            iterator begin() const;
            iterator end() const;

            size_t size() const;
        };

    }
}
}