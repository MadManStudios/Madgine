#pragma once

#include "global.h"

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT WorkGroupStorage {
        template <typename T>
        struct container_type {
            container_type() = default;
            container_type(const container_type &) = delete;
            container_type(container_type &&) = default;

            T *operator->()
            {
                return &mData[get()];
            }

            T &operator*()
            {
                return mData[get()];
            }

        private:
            std::map<WorkGroup *, T> mData;
        };

        static int registerLocalBssVariable(std::function<Any()> ctor);
        static void unregisterLocalBssVariable(int index);
        static int registerLocalObjectVariable(std::function<Any()> ctor);
        static void unregisterLocalObjectVariable(int index);

        static const Any &localVariable(int index);

        static void init(bool bss);
        static void finalize(bool bss);

        static WorkGroup *get();
    };

    template <typename T>
    using WorkgroupLocal = Global<T, WorkGroupStorage>;

}
}
