#pragma once

#include "Meta/keyvalue/virtualscope.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"
#include "Modules/threading/madgineobject.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "handlercollector.h"

#include "Madgine/debug/debuggablelifetime.h"

namespace Engine {

    struct MADGINE_HANDLER_EXPORT HandlerBase : VirtualScopeBase<>, Threading::MadgineObject<HandlerBase> {
        SERIALIZABLEUNIT(HandlerBase)

        HandlerBase(HandlerManager &ui);
        virtual ~HandlerBase() = default;

        virtual void onMouseVisibilityChanged(bool b);
                                                                
        virtual void startLifetime();
        void endLifetime();

        virtual std::string_view key() const = 0;

        template <typename T>
        T &getHandler()
        {
            return static_cast<T &>(getHandler(UniqueComponent::component_index<T>()));
        }

        HandlerBase &getHandler(size_t i);

        Threading::TaskQueue *viewTaskQueue() const;
        Threading::TaskQueue *modelTaskQueue() const;

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<HandlerBase>;

    protected:
        HandlerManager &mUI;

        DEBUGGABLE_LIFETIME(mLifetime);        
    };
}

REGISTER_TYPE(Engine::HandlerBase)