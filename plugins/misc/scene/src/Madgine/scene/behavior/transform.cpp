#include "../../scenelib.h"

#include "transform.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/nativebehaviorcollector.h"

NATIVE_BEHAVIOR(Rotate, Engine::Scene::rotate, Engine::Behavior::InputParameter<"Axis", Engine::Math::Vector3>, Engine::Behavior::InputParameter<"Speed", float>, Engine::Behavior::InputParameter<"Entity", Engine::Scene::EntityBinding>)