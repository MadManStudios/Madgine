#pragma once

#include "Meta/serialize/helper/annotations.h"
#include "Meta/reflect/helper/annotations.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Widgets, Widget, WidgetBase, Engine::Plugins::Constructor<Engine::Widgets::WidgetManager &, Engine::Widgets::WidgetBase *>, Engine::Serialize::TypeAnnotation, Engine::Reflect::TypeAnnotation)
