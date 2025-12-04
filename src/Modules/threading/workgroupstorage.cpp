#include "../moduleslib.h"

#include "workgroupstorage.h"

#include "globalvariablemanager.h"
#include "workgroup.h"

namespace Engine {
namespace Threading {

    std::shared_mutex &sMutex()
    {
        static std::shared_mutex mutex;
        return mutex;
    };

    static std::vector<GlobalVariableManager> &sWorkgroupLocalBssConstructors()
    {
        static std::vector<GlobalVariableManager> dummy;
        return dummy;
    }

    static std::vector<GlobalVariableManager> &sWorkgroupLocalObjectConstructors()
    {
        static std::vector<GlobalVariableManager> dummy;
        return dummy;
    }

    int WorkGroupStorage::registerLocalBssVariable(std::function<Any()> ctor)
    {
        std::unique_lock lock(sMutex());
        sWorkgroupLocalBssConstructors().emplace_back(std::move(ctor));
        return -static_cast<int>(sWorkgroupLocalBssConstructors().size());
    }

    void WorkGroupStorage::unregisterLocalBssVariable(int index)
    {
        std::shared_lock lock(sMutex());
        sWorkgroupLocalBssConstructors()[-(index + 1)].reset();
    }

    int WorkGroupStorage::registerLocalObjectVariable(std::function<Any()> ctor)
    {
        std::unique_lock lock(sMutex());
        sWorkgroupLocalObjectConstructors().emplace_back(std::move(ctor));
        return sWorkgroupLocalObjectConstructors().size() - 1;
    }

    void WorkGroupStorage::unregisterLocalObjectVariable(int index)
    {
        std::shared_lock lock(sMutex());
        sWorkgroupLocalObjectConstructors()[index].reset();
    }

    const Any &WorkGroupStorage::localVariable(int index)
    {
        size_t selfIndex = WorkGroup::self().mInstanceCounter;
        std::vector<GlobalVariableManager> &constructors = index < 0 ? sWorkgroupLocalBssConstructors() : sWorkgroupLocalObjectConstructors();
        if (index < 0)
            index = -(index + 1);
        std::shared_lock lock(sMutex());
        GlobalVariableManager &m = constructors[index];
        return m[selfIndex];
    }

    void WorkGroupStorage::init(bool bss)
    {
        size_t selfIndex = WorkGroup::self().mInstanceCounter;
        std::shared_lock lock(sMutex());
        if (bss) {
            for (GlobalVariableManager &m : sWorkgroupLocalBssConstructors()) {
                m[selfIndex];
            }
        } else {
            for (GlobalVariableManager &m : sWorkgroupLocalObjectConstructors()) {
                m[selfIndex];
            }
        }
    }

    void WorkGroupStorage::finalize(bool bss)
    {
        size_t selfIndex = WorkGroup::self().mInstanceCounter;
        std::shared_lock lock(sMutex());
        if (bss) {
            for (GlobalVariableManager &m : sWorkgroupLocalBssConstructors()) {
                m.remove(selfIndex);
            }
        } else {
            for (GlobalVariableManager &m : sWorkgroupLocalObjectConstructors()) {
                m.remove(selfIndex);
            }
        }
    }

    WorkGroup *WorkGroupStorage::get()
    {
        return &WorkGroup::self();
    }

}
}
