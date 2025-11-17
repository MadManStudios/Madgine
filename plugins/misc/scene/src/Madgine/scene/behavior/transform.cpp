#include "../../scenelib.h"

#include "transform.h"

#include "Madgine/behavior/behaviorerror.h"

#include "Madgine/behavior/nativebehaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

NATIVE_BEHAVIOR(Rotate, Engine::Scene::rotate, Engine::Behavior::InputParameter<"Axis", Engine::Vector3>, Engine::Behavior::InputParameter<"Speed", float>)