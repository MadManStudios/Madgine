#include "../resourceslib.h"

#include "resourceloader.h"

#include "resourcemanager.h"

namespace Engine {
namespace Resources {

    ResourceLoaderBase &getLoaderByIndex(size_t i)
    {
        return ResourceManager::getSingleton().get(i);
    }

    void waitForIOThread()
    {
        ResourceManager::getSingleton().waitForInit();
    }

    Threading::TaskFuture<bool> queueLoad(Threading::Task<bool> task, Threading::TaskQueue *queue)
    {
        return queue->queueTask(std::move(task));
    }

    Threading::TaskFuture<void> queueUnload(Threading::Task<void> task, Threading::TaskQueue *queue)
    {
        return queue->queueTask(std::move(task));
    }

}
}