#include "handlerlib.h"

#include "handlermanager.h"

#include "Generic/execution/execution.h"

#include "Meta/serialize/configs/controlled.h"

#include "Modules/debug/profiler/profile.h"
#include "Modules/threading/awaitables/awaitablesender.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/app/application.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "handler.h"

METATABLE_BEGIN(Engine::Behavior::HandlerManager)
    MEMBER(mHandlers)
METATABLE_END(Engine::Behavior::HandlerManager)

SERIALIZETABLE_BEGIN(Engine::Behavior::HandlerManager)
SERIALIZETABLE_END(Engine::Behavior::HandlerManager)

NAMED_UNIQUECOMPONENT(HandlerManager, Engine::Behavior::HandlerManager)

namespace Engine {
namespace Behavior {

    HandlerManager::HandlerManager(Window::MainWindow &window)
        : MainWindowComponent(window, 100)
        , mApp(window.app())
        , mHandlers(*this)
    {
    }

    HandlerManager::~HandlerManager()
    {
    }

    Threading::Task<bool> HandlerManager::init()
    {
        // Execution::detach(mgr.updatedSignal().connect([this] { onUpdate(); })); TODO

        if (!co_await MainWindowComponent::init())
            co_return false;

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            co_await handler->callInit();

        co_return true;
    }

    Threading::Task<void> HandlerManager::finalize()
    {
        assert(!mLifetime.running());

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            co_await handler->callFinalize();

        co_await MainWindowComponent::finalize();
    }

    void HandlerManager::startLifetime()
    {
        mWindow.lifetime().attach(mLifetime);

        for (const std::unique_ptr<HandlerBase> &handler : mHandlers)
            handler->startLifetime();
    }
        
    void HandlerManager::endLifetime()
    {
        mLifetime.end();
    }

    Debug::DebuggableLifetime<> &HandlerManager::lifetime()
    {
        return mLifetime;
    }

    App::Application &HandlerManager::app() const
    {
        return mApp;
    }

    bool HandlerManager::includeInLayout() const
    {
        return false;
    }

    void HandlerManager::hideCursor(bool keep)
    {
        if (!isCursorVisible())
            return;
        mKeepingCursorPos = keep;
        if (keep) {
            /*const OIS::MouseState &mouseState = mMouse->getMouseState();
                                mKeptCursorPosition = { (float)mouseState.X.abs, (float)mouseState.Y.abs };*/
        }
        // mGUI.hideCursor();

        for (const std::unique_ptr<HandlerBase> &h : mHandlers)
            h->onMouseVisibilityChanged(false);
    }

    void HandlerManager::showCursor()
    {
        if (isCursorVisible())
            return;
        if (mKeepingCursorPos) {
            /*OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
                                mutableMouseState.X.abs = mKeptCursorPosition.x;
                                mutableMouseState.Y.abs = mKeptCursorPosition.y;
                                callSafe([&]() {
                                        mouseMoved(OIS::MouseEvent(mMouse, mutableMouseState));*/
            // mGUI.showCursor();
            /*});*/
        } else {
            // mGUI.showCursor();
        }
        for (const std::unique_ptr<HandlerBase> &h : mHandlers)
            h->onMouseVisibilityChanged(true);
    }

    bool HandlerManager::isCursorVisible() const
    {
        return /* mGUI.isCursorVisible()*/ true;
    }

    std::set<HandlerBase *> HandlerManager::getHandlers()
    {
        std::set<HandlerBase *> result;
        for (const std::unique_ptr<HandlerBase> &h : mHandlers) {
            result.insert(h.get());
        }
        return result;
    }

    HandlerBase &HandlerManager::getHandler(size_t i)
    {
        return mHandlers.get(i);
    }

    Threading::TaskQueue *HandlerManager::viewTaskQueue() const
    {
        return mWindow.taskQueue();
    }

    Threading::TaskQueue *HandlerManager::modelTaskQueue() const
    {
        return mApp.taskQueue();
    }

}
}
