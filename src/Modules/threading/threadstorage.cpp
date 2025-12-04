#include "../moduleslib.h"

#if ENABLE_THREADING

#    include "Generic/any.h"

#    include "globalvariablemanager.h"
#    include "threadstorage.h"

#    if USE_PTHREAD_THREADLOCAL_STORE
#        include <pthread.h>
#    endif

namespace Engine {
namespace Threading {

    static std::shared_mutex &sMutex()
    {
        static std::shared_mutex dummy;
        return dummy;
    }

    static std::vector<GlobalVariableManager> &sLocalBssVariableConstructors()
    {
        static std::vector<GlobalVariableManager> dummy;
        return dummy;
    }

    static std::vector<GlobalVariableManager> &sLocalObjectVariableConstructors()
    {
        static std::vector<GlobalVariableManager> dummy;
        return dummy;
    }

    struct VariableStore {
        VariableStore()
        {
            mIndex = sCount.fetch_add(1);

            std::shared_lock lock { sMutex() };
            for (GlobalVariableManager &m : sLocalBssVariableConstructors()) {
                m[mIndex];
            }
        }

        void init()
        {
            std::shared_lock lock { sMutex() };
            for (GlobalVariableManager &m : sLocalObjectVariableConstructors()) {
                m[mIndex];
            }
        }

        size_t mIndex;
        static inline std::atomic<size_t> sCount = 0;
    };

#    if USE_PTHREAD_THREADLOCAL_STORE
    static pthread_key_t &sKey()
    {
        static pthread_key_t dummy;
        return dummy;
    }

    static VariableStore &sLocalVariables()
    {
        VariableStore *store = static_cast<VariableStore *>(pthread_getspecific(sKey()));
        assert(store);
        return *store;
    }

    static void destructTLS(void *store)
    {
        delete static_cast<VariableStore *>(store);
    }
#    else
    static VariableStore &sLocalVariables()
    {
        static thread_local VariableStore dummy;
        return dummy;
    }
#    endif

    int ThreadStorage::registerLocalBssVariable(std::function<Any()> ctor)
    {
        std::unique_lock lock { sMutex() };
        sLocalBssVariableConstructors().emplace_back(std::move(ctor));
        return -static_cast<int>(sLocalBssVariableConstructors().size());
    }

    void ThreadStorage::unregisterLocalBssVariable(int index)
    {
        std::shared_lock lock { sMutex() };
        sLocalBssVariableConstructors()[-(index + 1)].reset();
    }

    int ThreadStorage::registerLocalObjectVariable(std::function<Any()> ctor)
    {
        std::unique_lock lock { sMutex() };
        sLocalObjectVariableConstructors().emplace_back(std::move(ctor));
        return sLocalObjectVariableConstructors().size() - 1;
    }

    void ThreadStorage::unregisterLocalObjectVariable(int index)
    {
        std::shared_lock lock { sMutex() };
        sLocalObjectVariableConstructors()[index].reset();
    }

    const Any &ThreadStorage::localVariable(int index)
    {
        size_t self = sLocalVariables().mIndex;
        std::vector<GlobalVariableManager> &constructors = index < 0 ? sLocalBssVariableConstructors() : sLocalObjectVariableConstructors();
        if (index < 0)
            index = -(index + 1);
        std::shared_lock lock { sMutex() };
        GlobalVariableManager &m = constructors[index];
        return m[self];
    }

    void ThreadStorage::init(bool bss)
    {
        if (bss) {

#    if USE_PTHREAD_THREADLOCAL_STORE
            static int result = pthread_key_create(&sKey(), &destructTLS);
            assert(result == 0);

            pthread_setspecific(sKey(), new VariableStore);
#    endif

            size_t index = sLocalVariables().mIndex;

            std::shared_lock lock { sMutex() };
            for (GlobalVariableManager &m : sLocalBssVariableConstructors()) {
                m[index];
            }
        } else {
            sLocalVariables().init();
        }
    }

    void ThreadStorage::finalize(bool bss)
    {
        std::shared_lock lock { sMutex() };
        if (bss) {
            for (GlobalVariableManager &m : sLocalBssVariableConstructors()) {
                m.remove(sLocalVariables().mIndex);
            }
        } else {
            for (GlobalVariableManager &m : sLocalObjectVariableConstructors()) {
                m.remove(sLocalVariables().mIndex);
            }
        }
    }

}
}

#endif