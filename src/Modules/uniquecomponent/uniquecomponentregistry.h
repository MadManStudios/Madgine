#pragma once

#include "../plugins/binaryinfo.h"
#include "annotations.h"
#include "typeinfo.h"

namespace Engine {
namespace UniqueComponent {

    DERIVE_TYPENAME(VBase)

}
}

#if ENABLE_PLUGINS

namespace Engine {
namespace UniqueComponent {

    struct MODULES_EXPORT CollectorInfoBase {
        const Plugins::BinaryInfo *mBinary;
        std::vector<std::pair<std::vector<TypeInfo>, TypeInfo>> mElementInfos;
        IndexType<size_t> mBaseIndex;
        std::vector<std::string_view> mComponentNames;
    };

    MODULES_EXPORT std::vector<RegistryBase *> &registryRegistry();

    struct MODULES_EXPORT RegistryBase {
        RegistryBase(const TypeInfo &ti, const TypeInfo &namedTi, const Plugins::BinaryInfo *binary, const char * (*header)())
            : mBinary(binary)
            , mTi(ti)
            , mNamedTi(namedTi)
            , mHeader(header)
        {
            LOG("Adding: " << ti.type_name());
            registryRegistry().push_back(this);
        }

        ~RegistryBase()
        {
            std::erase(registryRegistry(), this);
        }

        virtual void onPluginLoad(const Plugins::BinaryInfo *) = 0;
        virtual void onPluginUnload(const Plugins::BinaryInfo *) = 0;

        const TypeInfo &type_info()
        {
            return mTi;
        }

        const TypeInfo &named_type_info()
        {
            return mNamedTi;
        }

        std::vector<CollectorInfoBase *>::iterator begin()
        {
            return mCollectors.begin();
        }

        std::vector<CollectorInfoBase *>::iterator end()
        {
            return mCollectors.end();
        }

        void addCollector(CollectorInfoBase *info)
        {
            mCollectors.push_back(info);
        }

        void removeCollector(CollectorInfoBase *info)
        {
            // assert(std::find(mLoadedCollectors.begin(), mLoadedCollectors.end(), info) == mLoadedCollectors.end());
            std::erase(mCollectors, info);
        }

        const Plugins::BinaryInfo *mBinary;
        const char *(*mHeader)();

        std::map<std::string_view, IndexType<uint32_t>> mComponentsByName;
        bool mIsNamed = false;

    protected:
        std::vector<CollectorInfoBase *> mCollectors;

    private:
        TypeInfo mTi;
        TypeInfo mNamedTi;
    };

    DLL_IMPORT_VARIABLE2(Registry, registry, typename Registry);

    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... _Annotations>
    struct Registry : RegistryBase {

        typedef _Base Base;
        using Annotations = GroupedAnnotation<typename replace<_Annotations>::template type<std::unique_ptr<Base>>...>;

        struct CollectorInfo : CollectorInfoBase {
            template <typename T, typename ActualType>
            size_t registerComponent(const TypeInfo &info, const TypeInfo &actualTi)
            {

                LOG("Registering Component: " << info.type_name());
                mComponents.emplace_back(type_holder<T>, type_holder<ActualType>);
                std::vector<TypeInfo> elementInfos;
                elementInfos.push_back(info);
                if constexpr (has_typename_VBase<T>) {
                    elementInfos.push_back(typeInfo<typename T::VBase>());
                }
                mElementInfos.emplace_back(std::move(elementInfos), actualTi);
                return mComponents.size() - 1;
            }

            void unregisterComponent(size_t i)
            {
                // mComponents[i] = nullptr; ??
                mElementInfos[i].first.clear();
            }

            std::vector<Annotations> mComponents;
        };

        Registry()
            : RegistryBase(TypeInfo { ti }, TypeInfo { namedTi }, &Plugins::PLUGIN_LOCAL(binaryInfo), header)
        {
        }

        static Registry &sInstance()
        {
            return registry<Registry<ti, namedTi, header, _Base, _Annotations...>>();
        }

        static std::vector<Annotations> &sComponents()
        {
            return sInstance().mComponents;
        }

        static const Annotations &get(size_t i)
        {
            return sInstance().mComponents[i];
        }

        static constexpr TypeInfo type_info()
        {
            return std::string_view { ti };
        }

        void onPluginLoad(const Plugins::BinaryInfo *bin)
        {
            assert(!bin->mIsStub);
            for (CollectorInfoBase *_info : mCollectors) {
                CollectorInfo *info = static_cast<CollectorInfo *>(_info);
                if (info->mBinary == bin) {
                    assert(!info->mBaseIndex);
                    info->mBaseIndex = mComponents.size();
                    for (const Annotations &annotations : info->mComponents) {
                        mComponents.push_back(annotations);
                    }
                }
            }
        }

        void onPluginUnload(const Plugins::BinaryInfo *bin)
        {
            for (CollectorInfoBase *_info : mCollectors) {
                CollectorInfo *info = static_cast<CollectorInfo *>(_info);
                if (info->mBinary == bin) {
                    assert(info->mBaseIndex);
                    mComponents.erase(mComponents.begin() + info->mBaseIndex, mComponents.begin() + info->mBaseIndex + info->mComponents.size());

                    for (CollectorInfoBase *i : mCollectors) {
                        if (i->mBaseIndex && i->mBaseIndex >= info->mBaseIndex)
                            i->mBaseIndex -= info->mComponents.size();
                    }

                    info->mBaseIndex.reset();
                }
            }
        }

    protected:
        static inline Registry *sSelf = &sInstance(); // Keep to ensure instantiation of registry, even with no component/collector in it

        std::vector<Annotations> mComponents;
    };

    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... _Annotations>
    struct NamedRegistry : Registry<ti, namedTi, header, _Base, _Annotations...> {

        struct CollectorInfo : Registry<ti, namedTi, header, _Base, _Annotations...>::CollectorInfo {
            template <typename T, typename ActualType>
            size_t registerComponent(const TypeInfo &ti2, const TypeInfo &actualTi)
            {
                this->mComponentNames.emplace_back(T::componentName());
                return Registry<ti, namedTi, header, _Base, _Annotations...>::CollectorInfo::template registerComponent<T, ActualType>(ti2, actualTi);
            }

            void unregisterComponent(size_t i)
            {
                Registry<ti, namedTi, header, _Base, _Annotations...>::CollectorInfo::unregisterComponent(i);
                this->mComponentNames[i] = {};
            }
        };

        NamedRegistry()
            : Registry<ti, namedTi, header, _Base, _Annotations...>()
        {
            this->mIsNamed = true;
        }

        static NamedRegistry &sInstance()
        {
            return static_cast<NamedRegistry &>(registry<Registry<ti, namedTi, header, _Base, _Annotations...>>());
        }

        static const std::map<std::string_view, IndexType<uint32_t>> &sComponentsByName()
        {
            return sInstance().mComponentsByName;
        }

        std::string_view componentName(uint32_t index)
        {
            for (CollectorInfoBase *info : this->mCollectors) {
                assert(index >= info->mBaseIndex);
                if (info->mBaseIndex && index < info->mBaseIndex + info->mComponentNames.size()) {
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

            for (CollectorInfoBase *info : this->mCollectors) {
                if (info->mBinary == bin) {
                    const std::vector<std::string_view> &names = static_cast<CollectorInfo *>(info)->mComponentNames;
                    for (std::string_view name : names) {
                        this->mComponentsByName[name] = counter;
                        ++counter;
                    }
                }
            }

            Registry<ti, namedTi, header, _Base, _Annotations...>::onPluginLoad(bin);
        }
    };

}
}

#else

namespace Engine {
namespace UniqueComponent {

    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... _Annotations>
    struct Registry {

        typedef _Base Base;
        using Annotations = GroupedAnnotation<typename replace<_Annotations>::template type<std::unique_ptr<Base>>...>;

        static std::vector<Annotations> sComponents();

        static Annotations get(size_t i)
        {
            return sComponents()[i];
        }
    };

    template <fixed_string ti, fixed_string namedTi, const auto &header, typename _Base, typename... _Annotations>
    struct NamedRegistry : Registry<ti, namedTi, header, _Base, _Annotations...> {

        static const std::map<std::string_view, IndexType<uint32_t>> &sComponentsByName();

        static std::string_view sComponentName(uint32_t index)
        {
            return std::ranges::find_if(sComponentsByName(), [=](const auto &v) { return v.second == index; })->first;
        }
    };

}
}
#endif