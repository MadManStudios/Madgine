#include "../uilib.h"
#include "handler.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"
#include "uimanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

DEFINE_UNIQUE_COMPONENT(Engine::Input, Handler)

METATABLE_BEGIN(Engine::Input::HandlerBase)
METATABLE_END(Engine::Input::HandlerBase)

namespace Engine {
namespace Input {
    HandlerBase::HandlerBase(UIManager &ui)
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
}
