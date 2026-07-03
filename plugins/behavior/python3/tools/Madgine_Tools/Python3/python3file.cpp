#include "python3toolslib.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "Meta/reflect/metatable_impl.h"

#include "Madgine_Tools/renderer/imroot.h"
#include "Madgine_Tools/texteditor/texteditor.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "python3editor.h"
#include "python3file.h"

METATABLE_BEGIN(Engine::Tools::Python3File)
METATABLE_END(Engine::Tools::Python3File)

namespace Engine {
namespace Tools {

    Python3File::Python3File(Python3Editor &editor, Behavior::Python3::Python3FileLoader::Resource *resource)
        : ResourceFile(editor, resource ? resource->path() : "")
        , mDocument(mPath, editor.root().getTool<TextEditor>())
    {
        
    }

    Python3File::~Python3File()
    {
    }

    void Python3File::saveAs(const Platform::Filesystem::Path &path)
    {
        /* Serialize::FileManager mgr { "Scene" };

        Serialize::FormattedSerializeStream stream = mgr.openWrite(path, Serialize::Formats::xml);

        Serialize::write(stream, mContainer, "Container");

        mPath = path;

        mHistory.onSave();*/
        throw 0;
    }

    void Python3File::render()
    {
        bool open = true;


        if (Begin(&open)) {
            if (editor().beginContent()) {
                mDocument.renderContent(ImGui::GetContentRegionAvail());
            }
            ImGui::End();

            mDocument.handleInputs();
        }
        ImGui::End();
        

        if (!open) {
            if (mHistory.isDirty()) {
                mEditor.root().dialogs().showGrouped("Close", closeDialog(), [this]() { mCloseRequested = true; });
            } else {
                mCloseRequested = true;
            }
        }
    }

}
}
