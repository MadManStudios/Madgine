#pragma once

#include "Interfaces/filesystem/path.h"

#include "Madgine_Tools/renderer/dialogs.h"
#include "Madgine_Tools/toolbase.h"

namespace Engine {
namespace Tools {

    struct MADGINE_RESOURCES_TOOLS_EXPORT ResourceEditor : ToolBase {
        ResourceEditor(ImRoot &root);

        void renderMenu() override;

        Threading::Task<bool> init(Resources::ResourceLoaderBase &loader, std::string type);

        virtual void open(Resources::ResourceBase *res) = 0;

        Dialog<Filesystem::Path> resourceFilePicker(bool allowNewFile = false, Filesystem::Path path = {}, Filesystem::Path selected = {});

        friend struct ResourceFileBase;

    private:
        using ToolBase::init;

        Resources::ResourceLoaderBase *mResourceLoader = nullptr;
        ResourcesTool *mManager = nullptr;
        std::string mType;        

        Dialog<Filesystem::Path> mSaveAsDialog;
    };

}
}