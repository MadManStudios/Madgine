#include "python3toolslib.h"

#include "python3immediatewindow.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "Interfaces/log/logsenders.h"

#include "Meta/keyvalue/valuetype.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/debug/debugger.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/texteditor/textdocument.h"
#include "Madgine_Tools/texteditor/texteditor.h"
#include "Python3/python3behaviors.h"
#include "Python3/python3env.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#if PY_MINOR_VERSION < 11
#    include <frameobject.h>
#else
#    define Py_BUILD_CORE
#    include "internal/pycore_frame.h"
#endif

METATABLE_BEGIN_BASE(Engine::Tools::Python3ImmediateWindow, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Python3ImmediateWindow)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Python3ImmediateWindow, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Python3ImmediateWindow)

UNIQUECOMPONENT(Engine::Tools::Python3ImmediateWindow)

namespace Engine {
namespace Tools {

    std::vector<TypedPtr> visualizeBehaviorDebugLocation(ContinuationList &continuations, DebuggerView &view, const Debug::ContextInfo &context, const PyObject *location, TypedPtr inlineLocation);

    Python3ImmediateWindow::Python3ImmediateWindow(ImRoot &root)
        : Tool<Python3ImmediateWindow>(root)
    {
    }

    std::string_view Python3ImmediateWindow::key() const
    {
        return "Python3ImmediateWindow";
    }

    Threading::Task<bool> Python3ImmediateWindow::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeBehaviorDebugLocation>();

        Debug::Debugger::getSingleton().addListener(this);

        mEnv = &Behavior::Python3::Python3Environment::getSingleton();

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> Python3ImmediateWindow::finalize()
    {
        Debug::Debugger::getSingleton().removeListener(this);

        co_await ToolBase::finalize();
    }

    void Python3ImmediateWindow::renderMenu()
    {
        ToolBase::renderMenu();
    }

    void Python3ImmediateWindow::render()
    {
        if (beginToolPanel("Python3 Immediate Window", &mVisible, ImGuiDir_Right)) {

            if (!mPrompt)
                mPrompt = std::make_unique<InteractivePrompt>(&getTool<TextEditor>(), this);

            mPrompt->render();
        }
        ImGui::End();
    }

    std::string_view Python3ImmediateWindow::name()
    {
        return "Python3ImmediateWindow";
    }

    bool Python3ImmediateWindow::wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line)
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

    void Python3ImmediateWindow::onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type)
    {
        /* if (const Behavior::Python3::Python3DebugLocation *pyLocation = location.as<const Behavior::Python3::Python3DebugLocation>()) {

            const Filesystem::Path &path = pyLocation->file();

            if (!path.empty()) {
                TextDocument &doc = getTool<TextEditor>().openDocument(path);
                doc.goToLine(pyLocation->lineNr());
            }
        }*/
    }

    bool Python3ImmediateWindow::interpret(std::string_view command)
    {
        ValueType retVal;
        KeyValueResult result = mEnv->execute(retVal, command, mPrompt.get());
        if (result) {
            Log::LogDummy { Log::MessageType::ERROR_TYPE, __FILE__, __LINE__, mPrompt.get() } << result;                        
        } else {
            Log::LogDummy { Log::MessageType::INFO_TYPE, __FILE__, __LINE__, mPrompt.get() } << retVal;     
        }
        mPrompt->resume();
        return false;
    }

}
}
