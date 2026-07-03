#include "python3toolslib.h"

#include "python3editor.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/debugger/debuggerview.h"

UNIQUECOMPONENT(Engine::Tools::Python3Editor);

METATABLE_BEGIN_BASE(Engine::Tools::Python3Editor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Python3Editor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Python3Editor, Engine::Tools::ToolBase)
// ENCAPSULATED_FIELD(Current, getCurrentName, load)
SERIALIZETABLE_END(Engine::Tools::Python3Editor)

namespace Engine {
namespace Tools {

    std::vector<TypedPtr> visualizeBehaviorDebugLocation(ContinuationList &continuations, DebuggerView &view, const Debug::ContextInfo &context, const PyObject *location, TypedPtr inlineLocation);

    Python3Editor::Python3Editor(ImRoot &root)
        : Tool<Python3Editor, ResourceEditor>(root)
    {
        mVisible = false;
    }

    Threading::Task<bool> Python3Editor::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeBehaviorDebugLocation>();

        Debug::Debugger::getSingleton().addListener(this);

        /* #if MODULES_HAS_THREADS
                registerNodeGraphEditorTests(mRoot.testEngine());
        #endif*/

        co_return co_await ResourceEditor::init(Behavior::Python3::Python3FileLoader::getSingleton(), "Python3");
    }

    Threading::Task<void> Python3Editor::finalize()
    {
        Debug::Debugger::getSingleton().removeListener(this);

        co_await ToolBase::finalize();
    }

    void Python3Editor::update()
    {
        std::erase_if(mFiles, [&](std::pair<Behavior::Python3::Python3FileLoader::Resource *const, Python3File> &p) {
            if (p.second.mCloseRequested)
                return true;
            p.second.render();
            return false;
        });

        ResourceEditor::update();
    }

    void Python3Editor::render()
    {
        ResourceEditor::render();
    }

    std::string_view Python3Editor::key() const
    {
        return "Python3Editor";
    }

    void Python3Editor::open(Resources::ResourceBase *res)
    {
        Behavior::Python3::Python3FileLoader::Resource *pythonFile = static_cast<Behavior::Python3::Python3FileLoader::Resource *>(res);

        auto [it, b] = mFiles.try_emplace(pythonFile, *this, pythonFile);
        it->second.Focus();

    }

    Dialog<> Python3Editor::closeDialog()
    {
        for (Python3File &file : kvValues(mFiles)) {
            co_await file.closeDialog();
        }
        co_return {};
    }

    
    bool Python3Editor::wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line)
    {
        /*if (const Behavior::Python3::Python3DebugLocation *pyLocation = location.as<const Behavior::Python3::Python3DebugLocation>()) {

            const Filesystem::Path &path = pyLocation->file();

            if (!path.empty()) {
                TextDocument *doc = getTool<TextEditor>().getDocument(path);
                if (doc && doc->hasBreakpoint(line)) {
                    doc->goToLine(line);
                    return true;
                }
            }
        }*/

        return false;
    }

    void Python3Editor::onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type)
    {
        /* if (const Behavior::Python3::Python3DebugLocation *pyLocation = location.as<const Behavior::Python3::Python3DebugLocation>()) {

            const Filesystem::Path &path = pyLocation->file();

            if (!path.empty()) {
                TextDocument &doc = getTool<TextEditor>().openDocument(path);
                doc.goToLine(pyLocation->lineNr());
            }
        }*/
    }

}
}
