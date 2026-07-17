#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyTypeType;

        struct PyType {
            PyObject_HEAD 
                const TypeName *mType;
        };

    }
}
}
