#pragma once

#include "Generic/execution/sender.h"
#include "Generic/genericresult.h"
#include "Madgine/debug/debuggablesender.h"
#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct PyObjectFieldAccessor;
        struct PyDictPtr;

        struct PyObjectIterator;

        struct MADGINE_PYTHON3_EXPORT PyObjectPtr {

            PyObjectPtr() = default;
            PyObjectPtr(PyObject *object);
            PyObjectPtr(const PyObjectPtr &other);
            PyObjectPtr(PyObjectPtr &&other);
            ~PyObjectPtr();

            PyObjectPtr &operator=(const PyObjectPtr &other);
            PyObjectPtr &operator=(PyObjectPtr &&other);

            static PyObjectPtr fromBorrowed(PyObject *object);

            PyObjectPtr get(std::string_view name) const;
            PyObjectPtr call(const ArgumentList &args) const;
            PyObjectPtr call(const char *format, ...) const;
            PyObjectPtr call(const PyDictPtr &kwargs, const char *format, ...) const;
            PyObjectPtr call(const PyObjectPtr &args, const PyObjectPtr &kwargs) const;

            ExecutionSender callAsync() const;

            PyObjectFieldAccessor operator[](const PyObjectPtr &name) const;

            void reset();
            PyObject *release();

            explicit operator bool() const;
            operator PyObject *() const;
            PyObject **operator&();

            static PyObjectPtr None();

            PyObjectIterator begin() const;
            PyObjectIterator end() const;

        protected:
            PyObject *mObject = nullptr;
        };

        struct MADGINE_PYTHON3_EXPORT PyObjectFieldAccessor {
            PyObjectFieldAccessor &operator=(const PyObjectPtr &value);

            PyObjectPtr mObject;
            PyObjectPtr mKey;
        };

    }
}
}