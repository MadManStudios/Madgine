#pragma once

#include "uniquecomponentregistry.h"

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {
namespace UniqueComponent {

    template <typename _Registry>
    struct Collector {
        using Registry = _Registry;

        typedef typename Registry::Base Base;

        Collector()
        {
            mInfo.mRegistryInfo = &typeInfo<Registry>;
            mInfo.mBaseInfo = &typeInfo<Base>;
            Registry::sInstance().addCollector(&mInfo);
        }
        Collector(const Collector &) = delete;
        ~Collector()
        {
            Registry::sInstance().removeCollector(&mInfo);
        }

        void operator=(const Collector &) = delete;

        static Collector &PLUGIN_LOCAL(sInstance)();

        size_t size() const
        {
            return PLUGIN_LOCAL(sInstance)().mInfo.mComponents.size();
        }

    private:
        typename Registry::CollectorInfo mInfo;

    public:
        template <typename T, typename ActualType>
        struct ComponentRegistrator : IndexHolder {
            ComponentRegistrator()
                : IndexHolder { PLUGIN_LOCAL(sInstance)().mInfo.template registerComponent<T, ActualType>(), PLUGIN_LOCAL(sInstance)().mInfo.mBaseIndex }
            {
            }

            ~ComponentRegistrator()
            {
                PLUGIN_LOCAL(sInstance)().mInfo.unregisterComponent(mIndex);
            }
        };
    };

    template <typename Registry>
    Collector<Registry> &Collector<Registry>::PLUGIN_LOCAL(sInstance)()
    {
        static Collector dummy;
        dummy.mInfo.mBinary = &Plugins::PLUGIN_LOCAL(binaryInfo);
        return dummy;
    }

}
}

#else

namespace Engine {
namespace UniqueComponent {

    template <typename Registry>
    struct Collector {
        typedef typename Registry::Base Base;
    };

}
}

#endif
