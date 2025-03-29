#pragma once

#include "pylistptr.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

 
        struct PyObjectIterator {

            using iterator_category = std::input_iterator_tag;
            using value_type = std::pair<PyObject *, PyObjectPtr>;
            using difference_type = ptrdiff_t;
            using pointer = void;
            using reference = std::pair<PyObject *, PyObjectPtr>;

            PyObjectIterator() = default;
            PyObjectIterator(PyObjectPtr object);
            PyObjectIterator(PyObjectPtr object, PyListPtr::iterator it);

            PyObjectIterator &operator++();
            void operator++(int);
            std::pair<PyObject *, PyObjectPtr> operator*() const;
            bool operator!=(const PyObjectIterator &other) const;
            bool operator==(const PyObjectIterator &other) const;

        private:
            PyObjectPtr mObject;
            PyListPtr::iterator mIt;
        };
    
    }
}
}