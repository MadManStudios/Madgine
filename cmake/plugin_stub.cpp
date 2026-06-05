#include "Modules/uniquecomponent/uniquecomponentregistry.h"

using namespace Engine;

struct UniqueComponentStub {

    static Plugins::CollectorInfoBase &getCollector(Plugins::RegistryBase *reg)
    {
        static std::map<Plugins::RegistryBase *, Plugins::CollectorInfoBase> sMap;
        auto pib = sMap.try_emplace(reg);
        Plugins::CollectorInfoBase &collector = pib.first->second;
        if (pib.second) {
            collector.mBinary = &Engine::Plugins::PLUGIN_LOCAL(binaryInfo);
            reg->addCollector(&collector);
        }
        return collector;
    }

    static Plugins::RegistryBase *findRegistry(std::string_view name)
    {
        for (Plugins::RegistryBase *reg : Plugins::registryRegistry()) {
            if (StringUtil::startsWith(reg->type_info().mFullName, name)) {
                return reg;
            }
        }
        LOG_FATAL("Unable to find Registry called: " << name);
        throw 0;
    }

    UniqueComponentStub(const char *type, const char *baseName, const char *vBase)
    {

        Plugins::RegistryBase *reg = findRegistry(baseName);
        Plugins::CollectorInfoBase &collector = getCollector(reg);
        auto &infos = collector.mElementInfos.emplace_back(std::vector<Plugins::TypeInfo> {}, type);
        infos.first.emplace_back(type);
        if (vBase)
            infos.first.emplace_back(vBase);
    }

};

#define UNIQUECOMPONENT_STUB(Type, Registry, VBase) \
    UniqueComponentStub CONCAT2(stub_, __LINE__) { #Type, #Registry, #VBase };

PLUGIN_STUBS