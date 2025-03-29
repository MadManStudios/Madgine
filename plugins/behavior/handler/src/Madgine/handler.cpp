#include "handlerlib.h"

#include "Madgine/behavior.h"
#include "handler.h"
#include "Madgine/window/mainwindow.h"
#include "handlermanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

DEFINE_UNIQUE_COMPONENT(Engine, Handler)

METATABLE_BEGIN(Engine::HandlerBase)
METATABLE_END(Engine::HandlerBase)

namespace Engine {
    HandlerBase::HandlerBase(HandlerManager &ui)
        : mUI(ui)
        , mLifetime(&ui.lifetime())
    {
    }

    HandlerBase &HandlerBase::getHandler(size_t i)
    {
        return mUI.getHandler(i);
    }

    Threading::Task<bool> HandlerBase::init()
    {
        co_return true;
    }

    Threading::Task<void> HandlerBase::finalize()
    {
        co_return;
    }

    void HandlerBase::onMouseVisibilityChanged(bool b)
    {
    }

    void HandlerBase::startLifetime()
    {
        mUI.lifetime().attach(mLifetime);
    }

    void HandlerBase::endLifetime()
    {
        mLifetime.end();
    }

    Threading::TaskQueue *HandlerBase::viewTaskQueue() const
    {
        return mUI.viewTaskQueue();
    }

}
