#pragma once

#include "Platform/filesystem/path.h"

#include "Madgine_Tools/renderer/dialogs.h"
#include "Madgine_Tools/toolbase.h"

namespace Engine {
namespace Tools {

    struct MADGINE_RESOURCES_TOOLS_EXPORT ResourceEditor : ToolBase {
        ResourceEditor(ImRoot &root);

        void renderMenu() override;

        Threading::Task<bool> init(Resources::ResourceLoaderBase &loader, std::string type);

        virtual void open(Resources::ResourceBase *res) = 0;

        Dialog<Platform::Filesystem::Path> resourceFilePicker(bool allowNewFile = false, Platform::Filesystem::Path path = {}, Platform::Filesystem::Path selected = {});

        friend struct ResourceFileBase;

    private:
        using ToolBase::init;

        Resources::ResourceLoaderBase *mResourceLoader = nullptr;
        ResourcesTool *mManager = nullptr;
        std::string mType;        

        Dialog<Platform::Filesystem::Path> mSaveAsDialog;
    };

}
}