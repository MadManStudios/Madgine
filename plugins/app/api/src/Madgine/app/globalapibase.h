#pragma once

#include "Modules/threading/madgineobject.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace App {
    struct MADGINE_APP_EXPORT GlobalAPIBase : VirtualScopeBase<>, Threading::MadgineObject<GlobalAPIBase> {
        GlobalAPIBase(Application &app);
        virtual ~GlobalAPIBase() = default;

        virtual std::string_view key() const = 0;

        template <typename T>
        T &getGlobalAPIComponent()
        {
            return static_cast<T &>(getGlobalAPIComponent(UniqueComponent::component_index<T>()));
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

REGISTER_TYPE(Engine::App::GlobalAPIBase)