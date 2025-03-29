#pragma once


#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Render, RenderContext, RenderContext, Engine::UniqueComponent::Constructor<Threading::TaskQueue *>)
