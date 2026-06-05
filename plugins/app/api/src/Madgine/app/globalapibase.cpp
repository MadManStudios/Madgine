#include "../applib.h"

#include "globalapibase.h"

#include "Meta/reflect/metatable_impl.h"

#include "application.h"

METATABLE_BEGIN(Engine::Core::GlobalAPIBase)
METATABLE_END(Engine::Core::GlobalAPIBase)

namespace Engine {
namespace Core {

    GlobalAPIBase::GlobalAPIBase(Application &app)
        : mApp(app)
    {
    }

    Threading::TaskQueue *GlobalAPIBase::taskQueue() const
    {
        return mApp.taskQueue();
    }

    void GlobalAPIBase::startLifetime()
    {
    }

    Application &GlobalAPIBase::app()
    {
        return mApp;
    }

    Threading::Task<bool> GlobalAPIBase::init()
    {
        co_return true;
    }

    Threading::Task<void> GlobalAPIBase::finalize()
    {
        co_return;
    }

    GlobalAPIBase &GlobalAPIBase::getGlobalAPIComponent(size_t i)
    {
        return mApp.getGlobalAPIComponent(i);
    }

}
}
