#pragma once

#include "uniquecomponentregistry.h"

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {
namespace Plugins {

    template <typename _Registry>
    struct Collector {
        using Registry = _Registry;

        typedef typename Registry::Base Base;

        Collector()
        {
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

        static void addInitializer(Closure<void()> initializer)
        {
            PLUGIN_LOCAL(sInstance)().mInfo.mInitializers.push_back(std::move(initializer));
        }

    private:
        typename Registry::CollectorInfo mInfo;

    public:
        template <typename T, typename ActualType>
        struct ComponentRegistrator : IndexHolder {
            ComponentRegistrator(std::string_view ti, std::string_view actualTi)
                : IndexHolder { PLUGIN_LOCAL(sInstance)().mInfo.template registerComponent<T, ActualType>(ti, actualTi), PLUGIN_LOCAL(sInstance)().mInfo.mBaseIndex }
            {
            }

            ~ComponentRegistrator()
            {
                PLUGIN_LOCAL(sInstance)
                ().mInfo.unregisterComponent(mIndex);
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
namespace Plugins {

    template <typename _Registry>
    struct Collector {
        using Registry = _Registry;
        typedef typename Registry::Base Base;
    };

}
}

#endif
