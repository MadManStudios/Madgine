#include "../../scenelib.h"

#include "scenesenders.h"

#include "Madgine/behavior/behaviorerror.h"

#include "Madgine/behavior/nativebehaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

NATIVE_BEHAVIOR(Yield_Simulation, Engine::Scene::yield_simulation)
NATIVE_BEHAVIOR(Wait_Simulation, Engine::Scene::wait_simulation, Engine::Behavior::InputParameter<"Duration", std::chrono::steady_clock::duration>)
