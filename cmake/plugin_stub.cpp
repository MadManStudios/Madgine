#include "Modules/uniquecomponent/uniquecomponentregistry.h"

using namespace Engine;

struct UniqueComponentStub {

    static UniqueComponent::CollectorInfoBase &getCollector(UniqueComponent::RegistryBase *reg, const TypeInfo *baseInfo)
    {
        static std::map<UniqueComponent::RegistryBase *, UniqueComponent::CollectorInfoBase> sMap;
        auto pib = sMap.try_emplace(reg);
        UniqueComponent::CollectorInfoBase &collector = pib.first->second;
        if (pib.second) {
            collector.mBinary = &Engine::Plugins::PLUGIN_LOCAL(binaryInfo);
            collector.mBaseInfo = baseInfo;
            collector.mRegistryInfo = reg->named_type_info();
            reg->addCollector(&collector);
        }
        return collector;
    }

    static UniqueComponent::RegistryBase *findRegistry(std::string_view name)
    {
        for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
            if (StringUtil::startsWith(reg->type_info()->mFullName, name)) {
                return reg;
            }
        }
        throw 0;
    }

    UniqueComponentStub(const char *type, const char *baseName, const char *vBase)
        : mBaseInfo(baseName, nullptr, type_holder<void>)
        , mInfo(type, nullptr, type_holder<void>)
        , mVBaseInfo(vBase, nullptr, type_holder<void>)
    {

        UniqueComponent::RegistryBase *reg = findRegistry(baseName);
        UniqueComponent::CollectorInfoBase &collector = getCollector(reg, &mBaseInfo);
        auto &infos = collector.mElementInfos.emplace_back();
        infos.first.emplace_back(&mInfo);
        if (vBase)
            infos.first.emplace_back(&mVBaseInfo);
        infos.second = &mInfo;
    }

    TypeInfo mBaseInfo;
    TypeInfo mInfo;
    TypeInfo mVBaseInfo;
};

#define UNIQUECOMPONENT_STUB(Type, Registry, VBase) \
    UniqueComponentStub CONCAT2(stub_, __LINE__) { #Type, #Registry, #VBase };

PLUGIN_STUBS