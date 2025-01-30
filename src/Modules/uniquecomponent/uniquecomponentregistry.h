#pragma once

#include "../plugins/binaryinfo.h"
#include "annotations.h"

namespace Engine {
namespace UniqueComponent {

    DERIVE_TYPENAME(VBase)

}
}

#if ENABLE_PLUGINS

namespace Engine {
namespace UniqueComponent {

    struct MODULES_EXPORT CollectorInfoBase {
        const TypeInfo *mRegistryInfo;
        const TypeInfo *mBaseInfo;
        const Plugins::BinaryInfo *mBinary;
        std::vector<std::vector<const TypeInfo *>> mElementInfos;
        IndexType<size_t> mBaseIndex;
        std::vector<std::string_view> mComponentNames;
    };

    MODULES_EXPORT std::vector<RegistryBase *> &registryRegistry();

    struct MODULES_EXPORT RegistryBase {
        RegistryBase(const TypeInfo *ti, const Plugins::BinaryInfo *binary, const TypeInfo *namedTi = nullptr)
            : mBinary(binary)
            , mTi(ti)
            , mNamedTi(namedTi ? namedTi : ti)
        {
            LOG("Adding: " << ti->mTypeName);
            registryRegistry().push_back(this);
        }

        ~RegistryBase()
        {
            std::erase(registryRegistry(), this);
        }

        virtual void onPluginLoad(const Plugins::BinaryInfo *) = 0;
        virtual void onPluginUnload(const Plugins::BinaryInfo *) = 0;

        const TypeInfo *type_info()
        {
            return mTi;
        }

        const TypeInfo *named_type_info()
        {
            return mNamedTi;
        }

        std::vector<CollectorInfoBase *>::iterator begin()
        {
            return mLoadedCollectors.begin();
        }

        std::vector<CollectorInfoBase *>::iterator end()
        {
            return mLoadedCollectors.end();
        }

        const Plugins::BinaryInfo *mBinary;

        std::map<std::string_view, IndexType<uint32_t>> mComponentsByName;
        bool mIsNamed = false;

    protected:
        std::vector<CollectorInfoBase *> mLoadedCollectors;

    private:
        const TypeInfo *mTi;
        const TypeInfo *mNamedTi;
    };

    DLL_IMPORT_VARIABLE2(Registry, registry, typename Registry);

    template <typename _Base, typename... _Annotations>
    struct Registry : RegistryBase {

        typedef _Base Base;
        using Annotations = GroupedAnnotation<typename replace<_Annotations>::template type<std::unique_ptr<Base>>...>;

        struct CollectorInfo : CollectorInfoBase {

            template <typename T, typename ActualType>
            size_t registerComponent()
            {

                LOG("Registering Component: " << typeName<T>());
                mComponents.emplace_back(type_holder<T>, type_holder<ActualType>);
                std::vector<const TypeInfo *> elementInfos;
                elementInfos.push_back(&typeInfo<T>);
                if constexpr (has_typename_VBase<T>) {
                    elementInfos.push_back(&typeInfo<typename T::VBase>);
                }
                mElementInfos.emplace_back(std::move(elementInfos));
                return mComponents.size() - 1;
            }

            void unregisterComponent(size_t i)
            {
                //mComponents[i] = nullptr; ??
                mElementInfos[i].clear();
            }

            std::vector<Annotations> mComponents;
        };

        Registry(const TypeInfo *namedTi = nullptr)
            : RegistryBase(&typeInfo<Registry>, &Plugins::PLUGIN_LOCAL(binaryInfo), namedTi)
        {
        }

        static Registry &sInstance()
        {
            return registry<Registry<_Base, _Annotations...>>();
        }

        static std::vector<Annotations> &sComponents()
        {
            return sInstance().mComponents;
        }

        static const Annotations &get(size_t i)
        {
            return sInstance().mComponents[i];
        }

        void addCollector(CollectorInfo *info)
        {
            mUnloadedCollectors.push_back(info);
        }

        void onPluginLoad(const Plugins::BinaryInfo *bin)
        {
            for (auto it = mUnloadedCollectors.begin(); it != mUnloadedCollectors.end();) {
                CollectorInfo *info = *it;
                if (info->mBinary == bin) {
                    mLoadedCollectors.push_back(info);
                    info->mBaseIndex = mComponents.size();
                    for (const Annotations &annotations : info->mComponents) {
                        mComponents.push_back(annotations);
                    }
                    it = mUnloadedCollectors.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void removeCollector(CollectorInfoBase *info)
        {
            //assert(std::find(mLoadedCollectors.begin(), mLoadedCollectors.end(), info) == mLoadedCollectors.end());
            std::erase(mUnloadedCollectors, info);
        }

        void onPluginUnload(const Plugins::BinaryInfo *bin)
        {
            for (auto it = mLoadedCollectors.begin(); it != mLoadedCollectors.end();) {
                CollectorInfo *info = static_cast<CollectorInfo *>(*it);
                if (info->mBinary == bin) {
                    mUnloadedCollectors.push_back(info);
                    it = mLoadedCollectors.erase(it);
                    mComponents.erase(mComponents.begin() + info->mBaseIndex, mComponents.begin() + info->mBaseIndex + info->mComponents.size());

                    for (CollectorInfoBase *i : mLoadedCollectors) {
                        if (i->mBaseIndex >= info->mBaseIndex)
                            i->mBaseIndex -= info->mComponents.size();
                    }

                    info->mBaseIndex.reset();
                } else {
                    ++it;
                }
            }
        }

    protected:
        static inline Registry *sSelf = &sInstance(); //Keep to ensure instantiation of registry, even with no component/collector in it

        std::vector<Annotations> mComponents;

        std::vector<CollectorInfo *> mUnloadedCollectors;
    };

    template <typename _Base, typename... _Annotations>
    struct NamedRegistry : Registry<_Base, _Annotations...> {

        struct CollectorInfo : Registry<_Base, _Annotations...>::CollectorInfo {
            template <typename T, typename ActualType>
            size_t registerComponent()
            {
                this->mComponentNames.emplace_back(T::componentName());
                return Registry<_Base, _Annotations...>::CollectorInfo::template registerComponent<T, ActualType>();
            }

            void unregisterComponent(size_t i)
            {
                Registry<_Base, _Annotations...>::CollectorInfo::unregisterComponent(i);
                this->mComponentNames[i] = {};
            }
        };

        NamedRegistry()
            : Registry<_Base, _Annotations...>(&typeInfo<NamedRegistry>)
        {
            this->mIsNamed = true;
        }

        static NamedRegistry &sInstance()
        {
            return static_cast<NamedRegistry &>(registry<Registry<_Base, _Annotations...>>());
        }

        static const std::map<std::string_view, IndexType<uint32_t>> &sComponentsByName()
        {
            return sInstance().mComponentsByName;
        }

        std::string_view componentName(uint32_t index)
        {
            for (CollectorInfoBase *info : this->mLoadedCollectors) {
                assert(index >= info->mBaseIndex);
                if (index < info->mBaseIndex + info->mComponentNames.size()) {
                    return info->mComponentNames[index - info->mBaseIndex];
                }
            }
            throw 0;
        }

        static std::string_view sComponentName(uint32_t index)
        {
            return sInstance().componentName(index);
        }

        void onPluginLoad(const Plugins::BinaryInfo *bin)
        {
            size_t counter = this->mComponentsByName.size();

            for (typename Registry<_Base, _Annotations...>::CollectorInfo *info : this->mUnloadedCollectors) {
                if (info->mBinary == bin) {
                    const std::vector<std::string_view> &names = static_cast<CollectorInfo *>(info)->mComponentNames;
                    for (std::string_view name : names) {
                        this->mComponentsByName[name] = counter;
                        ++counter;
                    }
                }
            }

            Registry<_Base, _Annotations...>::onPluginLoad(bin);
        }
    };

}
}

#else

namespace Engine {
namespace UniqueComponent {

    template <typename _Base, typename... _Annotations>
    struct Registry {

        typedef _Base Base;
        using Annotations = GroupedAnnotation<typename replace<_Annotations>::template type<std::unique_ptr<Base>>...>;

        static std::vector<Annotations> sComponents();

        static Annotations get(size_t i)
        {
            return sComponents()[i];
        }
    };

    template <typename _Base, typename... _Annotations>
    struct NamedRegistry : Registry<_Base, _Annotations...> {

        static const std::map<std::string_view, IndexType<uint32_t>> &sComponentsByName();

        static std::string_view sComponentName(uint32_t index)
        {
            return std::ranges::find_if(sComponentsByName(), [=](const auto &v) { return v.second == index; })->first;
        }
    };

}
}
#endif