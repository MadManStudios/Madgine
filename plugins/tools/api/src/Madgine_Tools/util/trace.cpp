#include "../toolslib.h"

#include "trace.h"

namespace Engine {
namespace Tools {

    KeyValueResult Trace::follow()
    {
        return mTrace();
    }

}
}