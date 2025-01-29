#include "../../scenelib.h"

#include "transform.h"

#include "Madgine/behaviorerror.h"

#include "Madgine/nativebehaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

NATIVE_BEHAVIOR(Rotate, Engine::Scene::rotate, Engine::InputParameter<Engine::Vector3>, Engine::InputParameter<float>)