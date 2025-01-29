#pragma once

#include "Modules/threading/madgineobject.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Meta/math/rect2i.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Madgine/render/renderpass.h"

namespace Engine {
namespace Window {

    struct MADGINE_CLIENT_EXPORT MainWindowComponentBase : Serialize::VirtualSerializableDataBase<VirtualScopeBase<Render::RenderPass>>, Threading::MadgineObject<MainWindowComponentBase> {
        MainWindowComponentBase(MainWindow &window, int priority);
        virtual ~MainWindowComponentBase() = default;

        template <typename T>
        T &getWindowComponent()
        {
            return static_cast<T &>(getWindowComponent(component_index<T>()));
        }

        MainWindowComponentBase &getWindowComponent(size_t i);

        MainWindow &window() const;

        virtual std::string_view key() const = 0;

        virtual int priority() const override;

        Rect2i getScreenSpace() const;
        const Rect2i &getClientSpace() const;
        virtual Rect2i getChildClientSpace();

        virtual void onResize(const Rect2i &space);
        
        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual bool injectKeyPress(const Input::KeyEventArgs &arg) { return false; };
        virtual bool injectKeyRelease(const Input::KeyEventArgs &arg) { return false; }
        virtual bool injectPointerPress(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectPointerRelease(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectPointerMove(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectAxisEvent(const Input::AxisEventArgs &arg) { return false; }

        const int mPriority;

        Threading::TaskQueue *taskQueue() const;

        virtual bool includeInLayout() const;

        virtual void startLifetime();

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<MainWindowComponentBase>;

        MainWindow &mWindow;
        Rect2i mClientSpace;
    };

}
}

REGISTER_TYPE(Engine::Window::MainWindowComponentBase)