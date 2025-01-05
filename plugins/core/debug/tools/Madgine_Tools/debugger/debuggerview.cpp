#include "../debugtoolslib.h"

#include "debuggerview.h"

#include "Madgine_Tools/imguiicons.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/debug/debugger.h"

#include "Meta/keyvalue/valuetype.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "Madgine/debug/debuggablesender.h"

UNIQUECOMPONENT(Engine::Tools::DebuggerView);

METATABLE_BEGIN_BASE(Engine::Tools::DebuggerView, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::DebuggerView)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::DebuggerView, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::DebuggerView)

namespace Engine {
namespace Tools {

    DebuggerView::DebuggerView(ImRoot &root)
        : Tool<DebuggerView>(root)
        , mDebugger(Debug::Debugger::getSingleton())
    {
    }

    Threading::Task<bool> DebuggerView::init()
    {
        mInspector = &getTool<Inspector>();

        mDebugger.addListener(this);

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> DebuggerView::finalize()
    {
        mDebugger.removeListener(this);

        co_await ToolBase::finalize();
    }

    const Debug::DebugLocation *DebuggerView::visualizeDebugLocation(const Debug::ContextInfo *context, const Debug::DebugLocation *location, const Debug::DebugLocation *inlineLocation)
    {
        if (!location)
            return nullptr;

        if (const Execution::SenderLocation *senderLocation = dynamic_cast<const Execution::SenderLocation *>(location)) {
            const Debug::DebugLocation *subLocation = nullptr;

            auto visitor = [this, context, location, inlineLocation, &subLocation](const Execution::StateDescriptor &desc) {
                float startY = ImGui::GetCursorScreenPos().y;

                std::visit(overloaded { [](const Execution::State::Text &text) {
                                           ImGui::Text(text.mText);
                                       },
                               [](const Execution::State::Progress &progress) {
                                   ImGui::ProgressBar(progress.mRatio, ImVec2 { 100.0f, 10.0f }, "");
                               },
                               [](const Execution::State::BeginBlock &begin) {
                                   ImGui::BeginGroupPanel(begin.mName.data());
                               },
                               [](const Execution::State::EndBlock &end) {
                                   ImGui::EndGroupPanel();
                               },
                               [](const Execution::State::PushDisabled &) {
                                   ImGui::BeginDisabled();
                               },
                               [](const Execution::State::PopDisabled &) {
                                   ImGui::EndDisabled();
                               },
                               [this, context, location, inlineLocation, &subLocation](const Execution::State::SubLocation &) {
                                   subLocation = visualizeDebugLocation(context, location->mChild, inlineLocation);
                               },
                               [](const Execution::State::Breakpoint &bp) {
                                   float offset = 0.0f;
                                   switch (bp.mAlignment) {

                                   case Execution::State::Breakpoint::Alignment::Center:
                                       offset = -9.0f;
                                       break;
                                   case Execution::State::Breakpoint::Alignment::Bottom:
                                       offset = -18.0f;
                                       break;
                                   }
                                   DrawBreakpoint(ImGui::GetCursorScreenPos().y + offset);
                               } },
                    desc);

                //if (current && !location->mChild) {
                //    DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY) - 7.0f);
                //}
            };
            senderLocation->visit(CallableView<void(const Execution::StateDescriptor &)> { visitor });

            return subLocation;
        } else {
            for (auto debugVisualizer : mDebugLocationVisualizers) {
                auto [matched, child] = debugVisualizer(this, context, location, inlineLocation);
                if (matched)
                    return child;
            }
            ImGui::Text("Unknown ["s + typeid(*location).name() + "]");
            return nullptr;
        }
    }

    std::optional<Debug::ContinuationMode> DebuggerView::contextControls(Debug::ContextInfo &context)
    {
        std::optional<Debug::ContinuationMode> mode;
        if (context.alive()) {
            ImGui::PushID(&context);
            if (!context.isPaused())
                ImGui::BeginDisabled();
            if (ImGui::Button(IMGUI_ICON_PLAY)) {
                mode = context.resume();
            }
            ImGui::SameLine(0, 0);
            if (ImGui::Button(IMGUI_ICON_STEP)) {
                mode = context.step();
            }
            if (!context.isPaused())
                ImGui::EndDisabled();
            else
                ImGui::BeginDisabled();
            ImGui::SameLine(0, 0);
            if (ImGui::Button(IMGUI_ICON_PAUSE)) {
                mode = context.pause();
            }
            if (context.isPaused())
                ImGui::EndDisabled();
            ImGui::SameLine(0, 0);
            if (ImGui::Button(IMGUI_ICON_STOP)) {
                mode = context.stop();
            }
            
            ImGui::PopID();
        }
        return mode;
    }

    void DebuggerView::render()
    {
        if (ImGui::Begin("Debug Contexts")) {
            for (Debug::ContextInfo &info : mDebugger.infos()) {
                std::ostringstream descriptor;
                descriptor << "Context";
                if (!info.alive())
                    descriptor << " (dead)";
                else if (info.isPaused())
                    descriptor << " (paused)";
                if (ImGui::Selectable(descriptor.str().c_str(), mSelectedContext == &info)) {
                    setCurrentContext(info);
                }
            }
        }
        ImGui::End();

        std::optional<Debug::ContinuationMode> continuation;
        Debug::DebugLocation *prevSelected = mSelectedLocation;
        mSelectedLocation = nullptr;

        if (ImGui::Begin("Current Debug Context")) {
            if (!mSelectedContext) {
                ImGui::Text("No context selected!");
            } else {
                continuation = contextControls(*mSelectedContext);

                Debug::DebugLocation *location = mSelectedContext->mChild;
                while (location) {
                    if (!mSelectedLocation && prevSelected == location)
                        mSelectedLocation = location;
                    if (ImGui::Selectable(location->toString().c_str(), mSelectedLocation == location))
                        mSelectedLocation = location;
                    location = location->mChild;
                }

                renderDebugContext(mSelectedContext);
            }
        }
        ImGui::End();

        if (ImGui::Begin("Locals")) {
            if (!mSelectedContext) {
                ImGui::Text("No context selected!");
            } else if (!mSelectedContext->isPaused()) {
                ImGui::Text("Context is not paused!");
            } else if (!mSelectedLocation) {
                ImGui::Text("No frame selected!");
            } else {
                if (ImGui::BeginTable("locals", 2, ImGuiTableFlags_Resizable)) {

                    for (auto &[key, value] : mSelectedLocation->localVariables()) {
                        ValueType v = value;
                        if (mInspector->drawValue(key, v, value.isReference()).first)
                            value = v;
                    }

                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();

        if (continuation)
            mSelectedContext->continueExecution(*continuation);
    }

    void DebuggerView::renderMenu()
    {
        ToolBase::renderMenu();
    }

    void DebuggerView::renderDebugContext(const Debug::ContextInfo *context)
    {
        std::unique_lock guard { context->mMutex };
        if (BeginDebuggablePanel("Debug Context")) {
            const Debug::DebugLocation *child = visualizeDebugLocation(context, context->mChild, nullptr);
            assert(!child); //Parents that allow inline rendering need to take care of child rendering.
            EndDebuggablePanel();
        }
        if (context->isPaused()) {
            std::string arguments = context->getArguments();

            if (!arguments.empty()) {
                Debug::ContinuationType type = context->continuationType();
                switch (type) {
                case Debug::ContinuationType::Cancelled:
                case Debug::ContinuationType::Error:
                    ImGui::PushStyleColor(ImGuiCol_Border, { 1.0f, 0.0f, 0.0f, 1.0f });
                    //ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
                    break;
                case Debug::ContinuationType::Return:
                    ImGui::PushStyleColor(ImGuiCol_Border, { 0.0f, 0.78f, 1.0f, 1.0f });
                    //ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.78f, 1.0f, 1.0f });
                    break;
                }
                ImGui::BeginGroupPanel();
                ImGui::Text(arguments);
                ImGui::EndGroupPanel();
                if (type != Debug::ContinuationType::Flow) {
                    ImGui::PopStyleColor(1);
                }
            }
        }
    }

    void DebuggerView::setCurrentContext(Debug::ContextInfo &context)
    {
        mSelectedContext = &context;
    }

    void DebuggerView::onSuspend(Debug::ContextInfo &context, Debug::ContinuationType type)
    {
        setCurrentContext(context);
    }

    bool DebuggerView::wantsPause(const Debug::DebugLocation *location, Debug::ContinuationType type)
    {
        return false;
    }

    std::string_view DebuggerView::key() const
    {
        return "DebuggerView";
    }

    static float sDebugStartX;

    bool BeginDebuggablePanel(const char *name)
    {
        sDebugStartX = ImGui::GetCursorScreenPos().x;
        ImGui::Indent();
        return true;
    }

    void EndDebuggablePanel()
    {
        ImGui::Unindent();
    }

    void DrawDebugMarker(float y)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float x = sDebugStartX;
        y += 7.0f;
        draw_list->AddRectFilled({ x + 3.0f, y - 2.0f }, { x + 12.0f, y + 3.0f }, IM_COL32(255, 200, 10, 255));
        draw_list->AddTriangleFilled({ x + 12.0f, y - 5.0f }, { x + 12.0f, y + 5.0f }, { x + 17.0f, y }, IM_COL32(255, 200, 10, 255));
    }

    void DrawBreakpoint(float y)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float x = sDebugStartX;
        y += 7.0f;
        draw_list->AddCircle({ x + 7.0f, y }, 7.0f, IM_COL32(155, 155, 155, 155));        
    }
}
}
