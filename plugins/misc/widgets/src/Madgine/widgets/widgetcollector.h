#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Meta/serialize/helper/annotations.h"

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Widgets, Widget, WidgetBase, Engine::UniqueComponent::Constructor<Engine::Widgets::WidgetManager &, Engine::Widgets::WidgetBase *>, Engine::Serialize::TypeAnnotation)
