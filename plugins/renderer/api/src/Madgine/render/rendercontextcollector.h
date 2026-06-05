#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Render, RenderContext, RenderContext, Engine::Plugins::Constructor<Threading::TaskQueue *>)
