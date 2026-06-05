#pragma once

#include "Meta/reflect/virtualscope.h"
#include "Meta/math/rect2i.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Madgine/render/renderpass.h"

namespace Engine {
namespace Core {

    struct MADGINE_CLIENT_EXPORT MainWindowComponentBase : Serialize::VirtualSerializableDataBase<Reflect::VirtualScopeBase<Render::RenderPass>>, Threading::MadgineObject<MainWindowComponentBase> {
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

        Math::Rect2i getScreenSpace() const;
        const Math::Rect2i &getClientSpace() const;
        virtual Math::Rect2i getChildClientSpace();

        virtual void onResize(const Math::Rect2i &space);

        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual bool onWindowEvent(const Platform::Window::WindowEvent &arg) { return false; };

        virtual bool wantsSoftwareKeyboard() const { return false; }

        const int mPriority;

        Threading::TaskQueue *taskQueue() const;

        virtual bool includeInLayout() const;

        virtual void startLifetime();

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<MainWindowComponentBase>;

        MainWindow &mWindow;
        Math::Rect2i mClientSpace;
    };

}
}