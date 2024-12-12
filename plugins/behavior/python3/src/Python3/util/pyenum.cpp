#include "../python3lib.h"

#include "pyenum.h"

#include "pyobjectutil.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        PyTypeObject PyEnumType = {
            .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                           .tp_name
            = "Engine.Enum",
            .tp_basicsize = sizeof(PyEnum),
            .tp_itemsize = 0,
            .tp_dealloc = &PyDealloc<PyEnum, &PyEnum::mEnum>,
            .tp_str = &PyStr<PyEnum, &PyEnum::mEnum>,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_doc = "Python implementation of EnumHolder",
            .tp_new = PyType_GenericNew,
        };

    }
}
}