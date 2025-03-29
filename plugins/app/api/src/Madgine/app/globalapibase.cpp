#include "../applib.h"

#include "globalapibase.h"

#include "application.h"

#include "Meta/keyvalue/metatable_impl.h"


METATABLE_BEGIN(Engine::App::GlobalAPIBase)
METATABLE_END(Engine::App::GlobalAPIBase)

namespace Engine {
namespace App {
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

