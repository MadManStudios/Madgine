#pragma once

#include "Platform/filesystem/path.h"

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct FileBrowser : Tool<FileBrowser> {
        SERIALIZABLEUNIT(FileBrowser)

        FileBrowser(ImRoot &root);

        virtual void render() override;

        std::string_view key() const override;

    private:
        Platform::Filesystem::Path mCurrentPath = Platform::Filesystem::Path { "." }.absolute();
        Platform::Filesystem::Path mSelectedPath = Platform::Filesystem::Path { "." }.absolute();
    };

}
}
