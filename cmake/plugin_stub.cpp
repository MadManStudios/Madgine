#include "Modules/uniquecomponent/uniquecomponentregistry.h"

using namespace Engine;

struct UniqueComponentStub {

    static UniqueComponent::CollectorInfoBase &getCollector(UniqueComponent::RegistryBase *reg)
    {
        static std::map<UniqueComponent::RegistryBase *, UniqueComponent::CollectorInfoBase> sMap;
        auto pib = sMap.try_emplace(reg);
        UniqueComponent::CollectorInfoBase &collector = pib.first->second;
        if (pib.second) {
            collector.mBinary = &Engine::Plugins::PLUGIN_LOCAL(binaryInfo);
            reg->addCollector(&collector);
        }
        return collector;
    }

    static UniqueComponent::RegistryBase *findRegistry(std::string_view name)
    {
        for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
            if (StringUtil::startsWith(reg->type_info().mFullName, name)) {
                return reg;
            }
        }
        throw 0;
    }

    UniqueComponentStub(const char *type, const char *baseName, const char *vBase)
    {

        UniqueComponent::RegistryBase *reg = findRegistry(baseName);
        UniqueComponent::CollectorInfoBase &collector = getCollector(reg);
        auto &infos = collector.mElementInfos.emplace_back(std::vector<UniqueComponent::TypeInfo> {}, type);
        infos.first.emplace_back(type);
        if (vBase)
            infos.first.emplace_back(vBase);
    }

};

#define UNIQUECOMPONENT_STUB(Type, Registry, VBase) \
    UniqueComponentStub CONCAT2(stub_, __LINE__) { #Type, #Registry, #VBase };

PLUGIN_STUBS