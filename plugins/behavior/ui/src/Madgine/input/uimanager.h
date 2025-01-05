#pragma once

#include "handlercollector.h"

#include "Generic/keyvalue.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Modules/uniquecomponent/component_index.h"

#include "Meta/math/vector2.h"

#include "Generic/intervalclock.h"

#include "Modules/threading/madgineobject.h"

#include "Generic/execution/lifetime.h"

namespace Engine {
namespace Input {
    struct MADGINE_UI_EXPORT UIManager : Threading::MadgineObject<UIManager> {

        UIManager(App::Application &app, Window::MainWindow &window);
        UIManager(const UIManager &) = delete;

        ~UIManager();

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

        Execution::Lifetime<> &lifetime();

        App::Application &app() const;
        Window::MainWindow &window() const;

        void shutdown();

    private:
        App::Application &mApp;
        Window::MainWindow &mWindow;

        Execution::Lifetime<> mLifetime;

    public:
        HandlerContainer<std::set<Placeholder<0>, KeyCompare<Placeholder<0>>>> mHandlers;        

    private:
        Vector2 mKeptCursorPosition;
        bool mKeepingCursorPos = false;

    };
}
}

REGISTER_TYPE(Engine::Input::UIManager)