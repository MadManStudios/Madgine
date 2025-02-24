#include "../toolslib.h"

#include "pluginexporter.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/cli/parameter.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Modules/ini/inifile.h"

#include "Madgine/codegen/codegen_cpp.h"

#include "Modules/plugins/binaryinfo.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Modules/plugins/pluginmanager.h"

#include "pluginmanager.h"

#if ENABLE_PLUGINS

UNIQUECOMPONENT(Engine::Tools::PluginExporter)

METATABLE_BEGIN_BASE(Engine::Tools::PluginExporter, Engine::Root::RootComponentBase)
METATABLE_END(Engine::Tools::PluginExporter)

namespace Engine {
namespace Tools {

    CLI::Parameter<Filesystem::Path> exportPlugins { { "--export-plugins", "-ep" }, "", "If set, the pluginmanager will save the current plugin selection after the boot to the specified config file." };
    CLI::Parameter<Filesystem::Path> generatePluginsCode { { "--generate-plugins-code", "-epc" }, "", "If set, the pluginmanager will export a uniquecomponent configuration source file for the current plugin selection to the specified path." };

    PluginExporter::PluginExporter(Root::Root &root)
        : RootComponent(root)
    {
    }

    std::string_view PluginExporter::key() const
    {
        return "Plugin Exporter";
    }

    Threading::Task<int> PluginExporter::runTools()
    {
        if (!exportPlugins->empty()) {

            Filesystem::Path p = *exportPlugins;

            Filesystem::createDirectories(p.parentPath());
            Ini::IniFile file;
            LOG("Saving Plugins to '" << p << "'");
            Plugins::PluginManager::getSingleton().saveSelection(file, true);
            if (!file.saveToDisk(p))
                co_return -1;
        }
        if (!generatePluginsCode->empty()) {
            exportStaticComponentHeader(generatePluginsCode);
        }
        co_return 0;
    }

    std::string fixInclude(const char *pStr, const Plugins::BinaryInfo *binInfo)
    {
        Filesystem::Path p = pStr;
        if (p.isRelative())
            p = Filesystem::Path { BINARY_DIR } / p;
        return p.relative(binInfo->mSourceRoot).str();
    };

    void PluginExporter::exportStaticComponentHeader(const Filesystem::Path &outFile)
    {
        LOG("Exporting uniquecomponent configuration source file '" << outFile << "'");
        LOG_WARNING("TODO: Export in alphabetical order to prevent changes in the version control.");

        std::set<const Plugins::BinaryInfo *> binaries;

        for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
            binaries.insert(reg->mBinary);

            for (UniqueComponent::CollectorInfoBase *collector : *reg) {
                binaries.insert(collector->mBinary);
            }
        }

        CodeGen::CppFile file;

        file.include(0, "Modules/moduleslib.h");
        file.include(0, "Modules/uniquecomponent/uniquecomponentregistry.h");
        file.include(0, "Modules/uniquecomponent/uniquecomponent.h");

        for (const Plugins::BinaryInfo *bin : binaries) {
            if (strlen(bin->mPrecompiledHeaderPath)) {
                file.beginCondition("BUILD_"s + bin->mName);
                file.include(0, bin->mPrecompiledHeaderPath);
                file.endCondition("BUILD_"s + bin->mName);
            }
        }

        for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
            const Plugins::BinaryInfo *bin = reg->mBinary;
            file.beginCondition("BUILD_"s + bin->mName);
            file.include(1, fixInclude(reg->named_type_info()->mHeaderPath, bin));

            for (UniqueComponent::CollectorInfoBase *collector : *reg) {
                for (const std::pair<std::vector<const TypeInfo *>, const TypeInfo*> &typeInfos : collector->mElementInfos) {
                    const TypeInfo *ti = typeInfos.first.front();
                    if (ti != &typeInfo<PluginManager> && ti != &typeInfo<PluginExporter>) {
                        file.beginCondition("BUILD_"s + collector->mBinary->mName);
                        file.include(1, fixInclude(ti->mHeaderPath, collector->mBinary));
                        file.endCondition("BUILD_"s + collector->mBinary->mName);
                    }
                }
            }
            file.endCondition("BUILD_"s + bin->mName);
        }

        file.beginNamespace("Engine");

        for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
            file.beginCondition("BUILD_"s + reg->mBinary->mName);

            file << R"(template <>
std::vector<)"
                 << reg->type_info()->mFullName << "::Annotations> " << reg->type_info()->mFullName
                 << R"(::sComponents()
{
	return {
)";

            for (UniqueComponent::CollectorInfoBase *collector : *reg) {
                file.beginCondition("BUILD_"s + collector->mBinary->mName);
                for (const std::pair<std::vector<const TypeInfo *>, const TypeInfo*> &typeInfos : collector->mElementInfos) {
                    const TypeInfo *ti = typeInfos.first.front();
                    if (ti != &typeInfo<PluginManager> && ti != &typeInfo<PluginExporter>)
                        file << "		{ type_holder<"
                             << ti->mFullName << ">, type_holder<" << typeInfos.second->mFullName << "> },\n";
                }
                file.endCondition("BUILD_"s + collector->mBinary->mName);
            }

            file << R"(
	}; 
}
)";

            file << R"(
#    define ACC 0

)";

            for (UniqueComponent::CollectorInfoBase *collector : *reg) {
                file.beginCondition("BUILD_"s + collector->mBinary->mName);
                file << "constexpr size_t CollectorBaseIndex_"
                     << collector->mBaseInfo->mTypeName << "_"
                     << collector->mBinary->mName << " = ACC;\n";
                size_t i = 0;
                for (const std::pair<std::vector<const TypeInfo *>, const TypeInfo*> &typeInfos : collector->mElementInfos) {
                    if (typeInfos.first.front() != &typeInfo<PluginManager> && typeInfos.first.front() != &typeInfo<PluginExporter>) {
                        for (const TypeInfo *typeInfo : typeInfos.first) {
                            while (typeInfo) {
                                file << R"(template <>
size_t UniqueComponent::component_index<)"
                                     << typeInfo->mFullName
                                     << ">() { return CollectorBaseIndex_"
                                     << collector->mBaseInfo->mTypeName << "_"
                                     << collector->mBinary->mName << " + " << i
                                     << "; }\n";
                                typeInfo = typeInfo->mDecayType ? typeInfo->mDecayType : nullptr;
                            }
                        }
                        ++i;
                    }
                }
                file << "#        undef ACC\n"
                     << "#        define ACC CollectorBaseIndex_"
                     << collector->mBaseInfo->mTypeName << "_"
                     << collector->mBinary->mName << " + " << i << "\n";
                file.endCondition("BUILD_"s + collector->mBinary->mName);
            }

            file << "\n#    undef ACC\n\n";

            if (reg->mIsNamed) {
                file << R"(template <>
const std::map<std::string_view, IndexType<uint32_t>> &)"
                     << reg->named_type_info()->mFullName
                     << R"(::sComponentsByName()
{
    static std::map<std::string_view, IndexType<uint32_t>> mapping {
)";

                for (UniqueComponent::CollectorInfoBase *collector : *reg) {
                    file.beginCondition("BUILD_"s + collector->mBinary->mName);
                    size_t i = 0;
                    for (const std::pair<std::vector<const TypeInfo *>, const TypeInfo*> &typeInfos : collector->mElementInfos) {
                        const TypeInfo *ti = typeInfos.first.front();
                        if (ti != &typeInfo<PluginManager> && ti != &typeInfo<PluginExporter>)
                            file << R"(		{")" << collector->mComponentNames[i] << R"(", CollectorBaseIndex_)"
                                 << collector->mBaseInfo->mTypeName << "_"
                                 << collector->mBinary->mName << " + " << i++ << "},\n";
                    }
                    file.endCondition("BUILD_"s + collector->mBinary->mName);
                }

                file << R"(
	}; 
    return mapping;
}
)";
            }

            file.endCondition("BUILD_"s + reg->mBinary->mName);
        }

        file.endNamespace();

        std::ofstream stream(outFile.str());
        assert(stream);

        file.generate(stream);
    }

}
}

#endif