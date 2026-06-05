#include "../toolslib.h"

#include "trace.h"

namespace Engine {
namespace Tools {

    Reflect::Result Trace::follow()
    {
        return mTrace();
    }

}
}