#include "python3toolslib.h"

#include "Madgine/debug/senderlocation.h"

#include "Madgine_Tools/debugger/continuationlist.h"
#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/texteditor/texteditor.h"
#include "Python3/python3behaviors.h"
#include "Python3/util/pylistptr.h"
#include "Python3/util/pymoduleptr.h"
#include "Python3/util/pysender.h"
#include "Python3/util/python3lock.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

    std::vector<TypedPtr> visualizeBehaviorDebugLocation(ContinuationList &continuations, DebuggerView &view, const Debug::ContextInfo &context, const PyObject *location, TypedPtr inlineLocation)
    {
        if (!location)
            return {};

        Behavior::Python3::Python3Lock lock;

        std::vector<TypedPtr> children;

        Behavior::Python3::PyObjectPtr coro = Behavior::Python3::PyObjectPtr::fromBorrowed(const_cast<PyObject *>(location));

        ImGui::BeginGroupPanel(PyUnicode_AsUTF8(coro.get("__name__")));

        if (ImGui::BeginTable("Code", 2, ImGuiTableFlags_SizingFixedFit)) {

            ImGui::TableSetupColumn("Line", 0);
            ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch);

            Behavior::Python3::PyObjectPtr frame = coro.get("cr_frame");

            Behavior::Python3::PyModulePtr inspect { "inspect" };
            Behavior::Python3::PyObjectPtr sourcelines = PyObject_CallFunctionObjArgs(inspect.get("getsourcelines"), static_cast<PyObject *>(frame), NULL);
            Behavior::Python3::PyListPtr sources = Behavior::Python3::PyListPtr::fromBorrowed(PyTuple_GetItem(sourcelines, 0));
            size_t baseLine = PyLong_AsLong(PyTuple_GetItem(sourcelines, 1));
            size_t lineNr = PyLong_AsLong(frame.get("f_lineno"));

            ImGui::PushFont(view.getTool<TextEditor>().font());

            for (PyObject *line : sources) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(std::to_string(baseLine));
                ImGui::TableNextColumn();

                float startY = ImGui::GetCursorScreenPos().y;

                ImGui::Text("%s", PyUnicode_AsUTF8(line));

                bool set = context.getBreakpoint(frame, baseLine);
                if (Breakpoint(startY, ImGui::GetCursorScreenPos().y, &set))
                    context.setBreakpoint(frame, baseLine, set);

                if (baseLine == lineNr) {
                    DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY));

                    ImGui::PopFont();

                    Behavior::Python3::PyObjectPtr next = coro.get("cr_await");
                    Behavior::Python3::Python3Unlock unlock;
                    if (Py_IS_TYPE(next, &Behavior::Python3::PySenderStateType)) {
                        Behavior::Python3::SenderState &state = reinterpret_cast<Behavior::Python3::PySenderState *>(static_cast<PyObject *>(next))->mState;
                        if (state.mChild) {
                            if (BeginDebuggablePanel("Sender")) {
                                std::ranges::move(view.visualizeDebugLocation(continuations, context, state.mChild, location), std::back_inserter(children));
                                EndDebuggablePanel();
                            }
                        }
                    } else if (Py_IS_TYPE(next, &Behavior::Python3::PyDebugLineType)) {
                        Behavior::Python3::DebugLine &debugLine = reinterpret_cast<Behavior::Python3::PyDebugLine *>(static_cast<PyObject *>(next))->mLine;
                        continuations.controls(debugLine.mContinuation);
                    } else if (!Py_IsNone(next)) {
                        if (BeginDebuggablePanel("Frame")) {
                            std::ranges::move(view.visualizeDebugLocation(continuations, context, static_cast<PyObject *>(next), location), std::back_inserter(children));
                            EndDebuggablePanel();
                        }
                    }

                    ImGui::PushFont(view.getTool<TextEditor>().font());
                }

                baseLine++;
            }

            ImGui::PopFont();

            ImGui::EndTable();
        }

        ImGui::EndGroupPanel();

        return children;
    }

}
}