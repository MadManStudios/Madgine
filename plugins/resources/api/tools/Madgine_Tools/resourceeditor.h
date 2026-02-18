#pragma once

#include "Madgine_Tools/renderer/dialogs.h"
#include "Madgine_Tools/toolbase.h"

namespace Engine {
namespace Tools {

    struct MADGINE_RESOURCES_TOOLS_EXPORT ResourceEditor : ToolBase {
        ResourceEditor(ImRoot &root);

        void renderMenu() override;

        Threading::Task<bool> init(Resources::ResourceLoaderBase &loader, std::string type);

        virtual void open(Resources::ResourceBase *res) = 0;

        bool BeginResourceFile(const void *id, const Filesystem::Path &path, bool isDirty, Closure<void(const Filesystem::Path &)> save, bool *open = nullptr, ImGuiWindowFlags flags = 0);

    private:
        using ToolBase::init;

        Resources::ResourceLoaderBase *mResourceLoader = nullptr;
        ResourcesTool *mManager = nullptr;
        std::string mType;

        Dialog<Filesystem::Path> mSaveAsDialog;
    };

}
}