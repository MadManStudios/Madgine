#include "../roottoolslib.h"

#include "pluginexporter.h"

#include "Platform/filesystem/fsapi.h"

#include "Modules/plugins/inifile.h"
#include "Modules/plugins/binaryinfo.h"
#include "Modules/plugins/pluginmanager.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Madgine/cli/parameter.h"
#include "Madgine/codegen/codegen_cpp.h"

#include "Meta/reflect/metatable_impl.h"

#include "Madgine_Tools/pluginmanager/pluginmanager.h"

#if ENABLE_PLUGINS

UNIQUECOMPONENT(Engine::Tools::PluginExporter)

METATABLE_BEGIN_BASE(Engine::Tools::PluginExporter, Engine::Core::RootComponentBase)
METATABLE_END(Engine::Tools::PluginExporter)

namespace Engine {
namespace Tools {

    Core::Parameter<Platform::Filesystem::Path> exportPlugins { { "--export-plugins", "-ep" }, "", "If set, the pluginmanager will save the current plugin selection after the boot to the specified config file." };
    Core::Parameter<Platform::Filesystem::Path> generatePluginsCode { { "--generate-plugins-code", "-epc" }, "", "If set, the pluginmanager will export a uniquecomponent configuration source file for the current plugin selection to the specified path." };

    PluginExporter::PluginExporter(Core::Root &root)
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

            Platform::Filesystem::Path p = *exportPlugins;

            Platform::Filesystem::createDirectories(p.parentPath());
            Plugins::IniFile file;
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
        Platform::Filesystem::Path p = pStr;
        if (p.isRelative())
            p = Platform::Filesystem::Path { BINARY_DIR } / p;
        Platform::Filesystem::Path r = p.relative(binInfo->mSourceRoot);
        if (r.empty())
            LOG_ERROR("Include Path '" << p << "' is not relative to source root '" << binInfo->mSourceRoot << "'");
        return r.str();
    };

    void PluginExporter::exportStaticComponentHeader(const Platform::Filesystem::Path &outFile)
    {
        LOG("Exporting uniquecomponent configuration source file '" << outFile << "'");

        std::set<const Plugins::BinaryInfo *> binaries;

        for (Plugins::RegistryBase *reg : Plugins::registryRegistry()) {
            binaries.insert(reg->mBinary);

            for (Plugins::CollectorInfoBase *collector : *reg) {
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

        for (Plugins::RegistryBase *reg : Plugins::registryRegistry()) {
            LOG("Exporting Registry: " << reg->type_info().type_name());
            const Plugins::BinaryInfo *bin = reg->mBinary;
            file.beginCondition("BUILD_"s + bin->mName);
            file.include(1, fixInclude(reg->mHeader(), bin));
            file.endCondition("BUILD_"s + bin->mName);
        }

        for (Plugins::RegistryBase *reg : Plugins::registryRegistry()) {
            file.beginCondition("BUILD_"s + reg->mBinary->mName);
            for (Plugins::CollectorInfoBase *collector : *reg) {
                file.beginCondition("BUILD_"s + collector->mBinary->mName);
                for (const std::pair<std::vector<Plugins::TypeInfo>, Plugins::TypeInfo> &typeInfos : collector->mElementInfos) {
                    bool first = true;
                    for (const Plugins::TypeInfo &ti : typeInfos.first) {
                        if (ti.type_name() != "PluginManager" && ti.type_name() != "PluginExporter") {
                            std::string_view namespaceName = ti.namespaceName();
                            if (!namespaceName.empty())
                                file.beginNamespace(namespaceName);
                            file << "struct " << ti.type_name() << ";\n";
                            if (!namespaceName.empty())
                                file.endNamespace();
                            if (first) {
                                first = false;
                                file << "extern template " << reg->type_info().mFullName << "::Annotations::GroupedAnnotation(Engine::type_holder_t<" << ti.mFullName << ">, Engine::type_holder_t<" << typeInfos.second.mFullName << ">);\n";
                            }
                        }
                    }
                }
                file.endCondition("BUILD_"s + collector->mBinary->mName);
            }
            file.endCondition("BUILD_"s + reg->mBinary->mName);
        }

        file.beginNamespace("Engine");

        for (Plugins::RegistryBase *reg : Plugins::registryRegistry()) {
            file.beginCondition("BUILD_"s + reg->mBinary->mName);

            file << R"(template <>
std::vector<)"
                 << reg->type_info().mFullName << "::Annotations> " << reg->type_info().mFullName
                 << R"(::sComponents()
{
	return {
)";

            for (Plugins::CollectorInfoBase *collector : *reg) {
                file.beginCondition("BUILD_"s + collector->mBinary->mName);
                for (const std::pair<std::vector<Plugins::TypeInfo>, Plugins::TypeInfo> &typeInfos : collector->mElementInfos) {
                    const Plugins::TypeInfo &ti = typeInfos.first.front();
                    if (ti.type_name() != "PluginManager" && ti.type_name() != "PluginExporter")
                        file << "		{ type_holder<"
                             << ti.mFullName << ">, type_holder<" << typeInfos.second.mFullName << "> },\n";
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

            for (Plugins::CollectorInfoBase *collector : *reg) {
                file.beginCondition("BUILD_"s + collector->mBinary->mName);
                file << "constexpr size_t CollectorBaseIndex_"
                     << reg->type_info().type_name() << "_"
                     << collector->mBinary->mName << " = ACC;\n";
                size_t i = 0;
                for (const std::pair<std::vector<Plugins::TypeInfo>, Plugins::TypeInfo> &typeInfos : collector->mElementInfos) {
                    if (typeInfos.first.front().type_name() != "PluginManager" && typeInfos.first.front().type_name() != "PluginExporter") {
                        for (const Plugins::TypeInfo &typeInfo : typeInfos.first) {
                            file << R"(template <>
size_t Plugins::component_index<)"
                                 << typeInfo.mFullName
                                 << ">() { return CollectorBaseIndex_"
                                 << reg->type_info().type_name() << "_"
                                 << collector->mBinary->mName << " + " << i
                                 << "; }\n";
                        }
                        ++i;
                    }
                }
                file << "#        undef ACC\n"
                     << "#        define ACC CollectorBaseIndex_"
                     << reg->type_info().type_name() << "_"
                     << collector->mBinary->mName << " + " << i << "\n";
                file.endCondition("BUILD_"s + collector->mBinary->mName);
            }

            file << "\n#    undef ACC\n\n";

            if (reg->mIsNamed) {
                file << R"(template <>
const std::map<std::string_view, IndexType<uint32_t>> &)"
                     << reg->named_type_info().mFullName
                     << R"(::sComponentsByName()
{
    static std::map<std::string_view, IndexType<uint32_t>> mapping {
)";

                for (Plugins::CollectorInfoBase *collector : *reg) {
                    file.beginCondition("BUILD_"s + collector->mBinary->mName);
                    size_t i = 0;
                    for (const std::pair<std::vector<Plugins::TypeInfo>, Plugins::TypeInfo> &typeInfos : collector->mElementInfos) {
                        const Plugins::TypeInfo &ti = typeInfos.first.front();
                        if (ti.type_name() != "PluginManager" && ti.type_name() != "PluginExporter")
                            file << R"(		{")" << collector->mComponentNames[i] << R"(", CollectorBaseIndex_)"
                                 << reg->type_info().type_name() << "_"
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