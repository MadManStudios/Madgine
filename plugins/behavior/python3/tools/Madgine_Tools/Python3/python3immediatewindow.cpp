#include "python3toolslib.h"

#include "python3immediatewindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Python3/python3debugger.h"
#include "Python3/python3env.h"
#include "Python3/util/python3lock.h"

#include "Generic/execution/execution.h"

#include "Madgine/debug/debugger.h"

#include "Madgine_Tools/texteditor/texteditor.h"

#include "Generic/execution/algorithm.h"

#include "Madgine_Tools/texteditor/textdocument.h"

#include "Madgine_Tools/debugger/debuggerview.h"

#include "Interfaces/log/logsenders.h"

#include "Python3/util/pylistptr.h"
#include "Python3/util/pymoduleptr.h"

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

    const Debug::DebugLocation *visualizeDebugLocation(DebuggerView &view, const Debug::ContextInfo &context, const Scripting::Python3::Python3DebugLocation &location, const Debug::DebugLocation *inlineLocation)
    {
        Scripting::Python3::Python3Lock lock;
        ImGui::BeginGroupPanel(PyUnicode_AsUTF8(PyFrame_GetCode2(location.mFrame)->co_filename));
        if (ImGui::TreeNode("Code")) {

            if (ImGui::BeginTable("Code", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingFixedFit)) {

                ImGui::TableSetupColumn("Line", 0);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch);

                Scripting::Python3::PyModulePtr inspect { "inspect" };
                Scripting::Python3::PyObjectPtr sourcelines = PyObject_CallFunctionObjArgs(inspect.get("getsourcelines"), location.mFrame, NULL);
                Scripting::Python3::PyListPtr sources = Scripting::Python3::PyListPtr::fromBorrowed(PyTuple_GetItem(sourcelines, 0));
                size_t baseLine = PyLong_AsLong(PyTuple_GetItem(sourcelines, 1));

                ImGui::PushFont(view.getTool<TextEditor>().font());

                for (PyObject *line : sources) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(std::to_string(baseLine));
                    ImGui::TableNextColumn();

                    float startY = ImGui::GetCursorScreenPos().y;

                    ImGui::Text("%s", PyUnicode_AsUTF8(line));

                    if (baseLine == location.lineNr()) {
                        DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY) - 7.0f);
                    }

                    baseLine++;
                }

                ImGui::PopFont();
            }
            ImGui::EndTable();

            ImGui::TreePop();
        }

        ImGui::EndGroupPanel();

        return nullptr;
    }

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
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeDebugLocation>();

        Debug::Debugger::getSingleton().addListener(this);

        mEnv = &Scripting::Python3::Python3Environment::getSingleton();

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
        if (ImGui::Begin("Python3ImmediateWindow", &mVisible)) {

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

    bool Python3ImmediateWindow::wantsPause(const Debug::DebugLocation &location, Debug::ContinuationType type)
    {

        const Scripting::Python3::Python3DebugLocation *pyLocation = dynamic_cast<const Scripting::Python3::Python3DebugLocation *>(&location);

        if (pyLocation) {
            const Filesystem::Path &path = pyLocation->file();

            if (!path.empty()) {
                TextDocument *doc = getTool<TextEditor>().getDocument(path);
                if (doc && doc->hasBreakpoint(pyLocation->lineNr())) {
                    doc->goToLine(pyLocation->lineNr());
                    return true;
                }
            }
        }

        return false;
    }

    void Python3ImmediateWindow::onSuspend(Debug::ContextInfo &context, Debug::ContinuationType type)
    {
        const Scripting::Python3::Python3DebugLocation *pyLocation = dynamic_cast<const Scripting::Python3::Python3DebugLocation *>(context.currentLocation());

        if (pyLocation) {
            const Filesystem::Path &path = pyLocation->file();

            if (!path.empty()) {
                TextDocument &doc = getTool<TextEditor>().openDocument(path);
                doc.goToLine(pyLocation->lineNr());
            }
        }
    }

    bool Python3ImmediateWindow::interpret(std::string_view command)
    {
        Execution::detach(mEnv->execute(command)
            | Log::log_error()
            | Execution::finally([this]() {
                  mPrompt->resume();
              })
            | Log::with_log(mPrompt.get())
            | Execution::with_debug_location()
            | Execution::with_sub_debug_location(&Debug::Debugger::getSingleton().createContext()));
        return false;
    }

}
}
