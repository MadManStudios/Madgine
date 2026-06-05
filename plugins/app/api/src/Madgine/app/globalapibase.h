#pragma once

#include "Meta/reflect/virtualscope.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Core {

    struct MADGINE_APP_EXPORT GlobalAPIBase : Reflect::VirtualScopeBase<>, Threading::MadgineObject<GlobalAPIBase> {
        GlobalAPIBase(Application &app);
        virtual ~GlobalAPIBase() = default;

        virtual std::string_view key() const = 0;

        template <typename T>
        T &getGlobalAPIComponent()
        {
            return static_cast<T &>(getGlobalAPIComponent(Plugins::component_index<T>()));
        }

        GlobalAPIBase &getGlobalAPIComponent(size_t i);

        Application &app();

        Threading::TaskQueue *taskQueue() const;

        virtual void startLifetime();

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<GlobalAPIBase>;

        Application &mApp;
    };
}
}