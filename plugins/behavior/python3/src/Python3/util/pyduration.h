#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyDurationType;

        struct PyDuration {
            PyObject_HEAD
                Reflect::Duration64 mDuration;
        };

    }
}
}
