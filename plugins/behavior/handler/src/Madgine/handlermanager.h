#pragma once

#include "handlercollector.h"

#include "Generic/keyvalue.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Meta/math/vector2.h"

#include "Generic/intervalclock.h"

#include "Modules/threading/madgineobject.h"

#include "Madgine/debug/debuggablelifetime.h"

#include "Madgine/window/mainwindowlistener.h"

namespace Engine {
struct MADGINE_HANDLER_EXPORT HandlerManager : Threading::MadgineObject<HandlerManager>, Window::MainWindowListener {

        using Self = HandlerManager;

        HandlerManager(App::Application &app, Window::MainWindow &window);
        HandlerManager(const HandlerManager &) = delete;

        ~HandlerManager();

        void clear();

        void hideCursor(bool keep = true);
        void showCursor();
        bool isCursorVisible() const;

        //Scene::ContextMask currentContext();

        std::set<HandlerBase *> getHandlers();        

        static const constexpr int sMaxInitOrder = 4;

        std::string_view key() const;

        template <typename T>
        T &getHandler()
        {
            return static_cast<T &>(getHandler(UniqueComponent::component_index<T>()));
        }

        HandlerBase &getHandler(size_t i);

        Threading::TaskQueue *viewTaskQueue() const;

        Threading::TaskQueue *modelTaskQueue() const;

        Threading::Task<bool> init();
        Threading::Task<void> finalize();
                                                                
        void startLifetime();
        void endLifetime();

        Debug::DebuggableLifetime<> &lifetime();

        void onActivate(bool active) override;

        App::Application &app() const;
        Window::MainWindow &window() const;

        void shutdown();

    private:
        App::Application &mApp;
        Window::MainWindow &mWindow;

        DEBUGGABLE_LIFETIME(mLifetime);

    public:
        HandlerContainer<std::set<Placeholder<0>, KeyCompare<Placeholder<0>>>> mHandlers;        

    private:
        Vector2 mKeptCursorPosition;
        bool mKeepingCursorPos = false;

    };
}

REGISTER_TYPE(Engine::HandlerManager)