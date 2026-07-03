#pragma once

#include "Madgine_Tools/resources/resourcefile.h"
#include "Madgine_Tools/texteditor/textdocument.h"
#include "Python3/python3fileloader.h"

namespace Engine {
namespace Tools {

    struct Python3File : ResourceFile<Python3Editor> {

        Python3File(Python3Editor &editor, Behavior::Python3::Python3FileLoader::Resource *resource);
        ~Python3File();

        void saveAs(const Platform::Filesystem::Path &path) override;

        void render();

    private:
        TextDocument mDocument;
    };

}
}