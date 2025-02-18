#include "../../widgetslib.h"

#include "widgetsenders.h"


NATIVE_BEHAVIOR(Yield_Frame, Engine::Widgets::yield_frame)
NATIVE_BEHAVIOR(Wait_Frame, Engine::Widgets::wait_frame, Engine::InputParameter<std::chrono::steady_clock::duration>)
