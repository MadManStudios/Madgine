#pragma once

#include "Generic/intervalclock.h"
#include "Generic/keyvalue.h"

#include "Meta/math/vector2.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/uniquecomponent/component_index.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/debug/debuggablelifetime.h"
#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "handlercollector.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_HANDLER_EXPORT HandlerManager : Window::MainWindowComponent<HandlerManager> {

        using Self = HandlerManager;

        HandlerManager(Window::MainWindow &window);
        HandlerManager(const HandlerManager &) = delete;

        ~HandlerManager();

        void hideCursor(bool keep = true);
        void showCursor();
        bool isCursorVisible() const;

        std::set<HandlerBase *> getHandlers();

        template <typename T>
        T &getHandler()
        {
            return static_cast<T &>(getHandler(UniqueComponent::component_index<T>()));
        }

        HandlerBase &getHandler(size_t i);

        Threading::TaskQueue *viewTaskQueue() const;

        Threading::TaskQueue *modelTaskQueue() const;

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void startLifetime() override;
        void endLifetime();

        Debug::DebuggableLifetime<> &lifetime();

        App::Application &app() const;        

        bool includeInLayout() const override;

    private:
        App::Application &mApp;

        DEBUGGABLE_LIFETIME(mLifetime);

    public:
        HandlerContainer<std::set<Placeholder<0>, KeyCompare<Placeholder<0>>>> mHandlers;

    private:
        Vector2 mKeptCursorPosition;
        bool mKeepingCursorPos = false;
    };
}
}