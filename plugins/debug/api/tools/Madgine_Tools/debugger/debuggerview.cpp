#include "../debugtoolslib.h"

#include "debuggerview.h"

#include "Interfaces/debug/stacktrace.h"

#include "Meta/keyvalue/valuetype.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/debug/debuggablelifetime.h"
#include "Madgine/debug/debuggablesender.h"
#include "Madgine/debug/debugger.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

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

        mInspector->addPreviewDefinition<Debug::ContextInfo>([this](const Traced<Debug::ContextInfo *> &context) {
            renderDebugContext(*context.get());
            return false;
        });

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> DebuggerView::finalize()
    {
        mDebugger.removeListener(this);

        co_await ToolBase::finalize();
    }

    std::vector<TypedPtr> DebuggerView::visualizeDebugLocation(const Debug::ContextInfo &context, TypedPtr location, TypedPtr inlineLocation)
    {
        if (const Debug::SenderLocation *senderLocation = location.as<const Debug::SenderLocation>()) {
            std::vector<TypedPtr> subLocations;

            IndexType<size_t> breakpoint;
            bool isMarker = false;
            Debug::Continuation *continuation = nullptr;
            IndexType<size_t> *lineFeedback = nullptr;
            size_t breakpointIndex = 0;
            bool lastWasText = false;

            auto visitor = [&, inlineLocation, senderLocation](const Execution::StateDescriptor &desc) {
                float startY = ImGui::GetCursorScreenPos().y;

                bool actualContent = false;

                std::visit(overloaded { [&](const Execution::State::Text &text) {
                                           ImGui::TextWrapped("%s", text.mText.c_str());
                                           actualContent = true;
                                       },
                               [&](const Execution::State::Progress &progress) {
                                   ImGui::ProgressBar(progress.mRatio, ImVec2 { 100.0f, 10.0f }, "");
                                   actualContent = true;
                               },
                               [&](const Execution::State::BeginBlock &begin) {
                                   ImGui::BeginGroupPanel(begin.mName.data(), {}, begin.mCompleted ? IM_COL32(20, 255, 20, 15) : 0);
                                   actualContent = true;
                               },
                               [&](const Execution::State::EndBlock &end) {
                                   if (lastWasText)
                                       ImGui::Dummy({ 0.0f, ImGui::GetStyle().ItemSpacing.y });
                                   ImGui::EndGroupPanel();
                                   actualContent = true;
                               },
                               [](const Execution::State::PushDisabled &) {
                                   ImGui::BeginDisabled();
                               },
                               [](const Execution::State::PopDisabled &) {
                                   ImGui::EndDisabled();
                               },
                               [&, inlineLocation](const Execution::State::DebugLocation &subLoc) {
                                   std::ranges::move(visualizeDebugLocation(context, subLoc.mLocation, inlineLocation), std::back_inserter(subLocations));
                                   actualContent = true;
                               },
                               [&](const Execution::State::Breakpoint &bp) {
                                   if (bp.mContinuation) {
                                       assert(!continuation);
                                       continuation = &bp.mContinuation;

                                       std::stringstream ss;
                                       continuation->visitArguments(ss);
                                       std::string arguments = ss.str();
                                       if (!arguments.empty()) {
                                           Debug::ContinuationType type = continuation->type();
                                           switch (type) {
                                           case Debug::ContinuationType::Cancelled:
                                           case Debug::ContinuationType::Error:
                                               ImGui::PushStyleColor(ImGuiCol_Border, { 1.0f, 0.0f, 0.0f, 1.0f });
                                               // ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
                                               break;
                                           case Debug::ContinuationType::Return:
                                               ImGui::PushStyleColor(ImGuiCol_Border, { 0.0f, 0.78f, 1.0f, 1.0f });
                                               // ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.78f, 1.0f, 1.0f });
                                               break;
                                           case Debug::ContinuationType::Flow:
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
                                   lineFeedback = bp.mLineFeedback;
                                   breakpoint = breakpointIndex++;
                               },
                               [&](const Execution::State::Marker &m) {
                                   isMarker = true;
                               },
                               [](const Execution::State::FunctionPtr &f) {
                                   const char *name = "function";
                                   const char *details = f.mTypeName;
#ifndef NDEBUG
                                   Debug::TraceBack trace = Debug::resolveSymbols(&f.mFunctionPtr, 1)[0];
                                   std::string detailsStr = std::string { trace.mFile } + ":" + std::to_string(trace.mLineNr);

                                   name = trace.mFunction;
                                   details = detailsStr.c_str();
#endif

                                   ImGui::Text("%s", name);
                                   ImGui::SetItemTooltip("%s", details);
                               } },
                    desc);

                if (actualContent) {
                    lastWasText = std::holds_alternative<Execution::State::Text>(desc);
                    if (breakpoint) {
                        if (lineFeedback) {
                            *lineFeedback = breakpoint;
                        }
                        bool set = context.getBreakpoint(senderLocation, breakpoint);
                        if (Breakpoint(startY, ImGui::GetCursorScreenPos().y, &set))
                            context.setBreakpoint(senderLocation, breakpoint, set);
                        breakpoint.reset();
                        if (continuation) {
                            DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY));

                            ImGui::PushID(continuation);
                            ControlButton button = contextControls(false);
                            ImGui::PopID();
                            switch (button) {
                            case ControlButton::PLAY:
                                (*continuation)(const_cast<Debug::ContextInfo &>(context).resume());
                                break;
                            case ControlButton::STEP:
                                (*continuation)(const_cast<Debug::ContextInfo &>(context).step());
                                break;
                            case ControlButton::STOP:
                                (*continuation)(Debug::ContinuationMode::Abort);
                                break;
                            case ControlButton::PAUSE:
                                throw 0;
                                break;
                            case ControlButton::NONE:
                                break;
                            }

                            continuation = nullptr;
                        }
                    }
                    if (isMarker) {
                        isMarker = false;
                        DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY));
                    }
                }
            };
            senderLocation->visit(visitor);

            return subLocations;
        } else {
            auto it = mDebugLocationVisualizers.find(location.type());
            if (it != mDebugLocationVisualizers.end()) {
                return it->second(*this, context, location.ptr(), inlineLocation);
            }
            ImGui::TextWrapped("Unknown [%s]", location.type().name());
            return {};
        }
    }

    DebuggerView::ControlButton DebuggerView::contextControls(bool running)
    {
        ControlButton button = ControlButton::NONE;
        if (running)
            ImGui::BeginDisabled();
        if (ImGui::Button(IMGUI_ICON_PLAY)) {
            button = ControlButton::PLAY;
        }
        ImGui::SameLine(0, 0);
        if (ImGui::Button(IMGUI_ICON_STEP)) {
            button = ControlButton::STEP;
        }
        if (running)
            ImGui::EndDisabled();
        else
            ImGui::BeginDisabled();
        ImGui::SameLine(0, 0);
        if (ImGui::Button(IMGUI_ICON_PAUSE)) {
            button = ControlButton::PAUSE;
        }
        if (!running)
            ImGui::EndDisabled();
        ImGui::SameLine(0, 0);
        if (ImGui::Button(IMGUI_ICON_STOP)) {
            button = ControlButton::STOP;
        }

        return button;
    }

    void DebuggerView::render()
    {
        if (beginToolWindow("Debugger", &mVisible)) {

            if (beginSubPanel("Debug Contexts", nullptr, ImGuiDir_Left)) {
                std::unique_lock lock { mDebugger.mMutex };
                for (Debug::ContextInfo &info : mDebugger.infos()) {
                    std::ostringstream descriptor;
                    if (info.alive()) {
                        descriptor << "Context";
                        if (info.isPaused()) {
                            descriptor << " (paused)";
                        }
                    } else {
                        descriptor << " (dead)";
                    }

                    descriptor << "###Context";
                    ImGui::PushID(&info);
                    if (ImGui::Selectable(descriptor.str().c_str(), mSelectedContext == &info)) {
                        setCurrentContext(info);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::End();

            Debug::DebugLocation *prevSelected = mSelectedLocation;
            mSelectedLocation = nullptr;

            if (beginContent()) {
                if (!mSelectedContext) {
                    ImGui::Text("No context selected!");
                } else {
                    ControlButton button = contextControls(!mSelectedContext->isPaused());
                    switch (button) {
                    }

                    // TODO: Build tree
                    /* Debug::DebugLocation *location = mSelectedContext->mChild;
                    while (location) {
                        if (!mSelectedLocation && prevSelected == location)
                            mSelectedLocation = location;
                        if (ImGui::Selectable(location->toString().c_str(), mSelectedLocation == location))
                            mSelectedLocation = location;
                        location = location->mChild;
                    }*/

                    renderDebugContext(*mSelectedContext);
                }
            }
            ImGui::End();

            if (beginSubPanel("Locals", nullptr, ImGuiDir_Down)) {
                if (!mSelectedContext) {
                    ImGui::Text("No context selected!");
                } else if (!mSelectedContext->isPaused()) {
                    ImGui::Text("Context is not paused!");
                } else if (!mSelectedLocation) {
                    ImGui::Text("No frame selected!");
                } else {
                    if (ImGui::BeginTable("locals", 2, ImGuiTableFlags_Resizable)) {

                        /* for (auto &[key, value] : mSelectedLocation->localVariables()) {
                            ValueType v = value;
                            if (mInspector->drawValue(key, v, value.isReference(), value.type()).first)
                                value = v;
                        }*/
                        ImGui::Text("TODO");

                        ImGui::EndTable();
                    }
                }
            }
            ImGui::End();
        }
        ImGui::End();
    }

    void DebuggerView::renderMenu()
    {
        ToolBase::renderMenu();
    }

    void DebuggerView::renderDebugContext(const Debug::ContextInfo &context)
    {
        std::unique_lock guard { context.mMutex };
        if (context.mChild) {
            if (BeginDebuggablePanel("Debug Context")) {
                [[maybe_unused]] std::vector<TypedPtr> children = visualizeDebugLocation(context, context.mChild, {});
                assert(children.empty()); // Parents that allow inline rendering need to take care of child rendering.
                EndDebuggablePanel();
            }
        }
    }

    void DebuggerView::renderLifetime(Debug::DebuggableLifetimeBase &lifetime)
    {
        for (Debug::ContextInfo &context : lifetime.debugContexts()) {
            ControlButton control = contextControls(!context.isPaused());
            renderDebugContext(context);
        }
    }

    void DebuggerView::setCurrentContext(Debug::ContextInfo &context)
    {
        mSelectedContext = &context;
    }

    void DebuggerView::onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type)
    {
        setCurrentContext(context);
    }

    bool DebuggerView::wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line)
    {
        return false;
    }

    std::string_view DebuggerView::key() const
    {
        return "DebuggerView";
    }

    static std::vector<float> sDebugStartX;

    bool BeginDebuggablePanel(const char *name)
    {
        sDebugStartX.push_back(ImGui::GetCursorScreenPos().x);
        ImGui::Indent();
        return true;
    }

    void EndDebuggablePanel()
    {
        ImGui::Unindent();
        sDebugStartX.pop_back();
    }

    void DrawDebugMarker(float y)
    {
        ImGui::PushClipRect({-1000, -1000}, {10000, 10000}, false);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float x = sDebugStartX.back();
        draw_list->AddRectFilled({ x + 3.0f, y - 2.0f }, { x + 12.0f, y + 3.0f }, IM_COL32(255, 200, 10, 255));
        draw_list->AddTriangleFilled({ x + 12.0f, y - 5.0f }, { x + 12.0f, y + 5.0f }, { x + 17.0f, y }, IM_COL32(255, 200, 10, 255));
        ImGui::PopClipRect();
    }

    bool Breakpoint(float startY, float endY, bool *set)
    {
        const float radius = 7.0f;
        float x = sDebugStartX.back() + radius + 3.0f;
        float y = 0.5f * (endY + startY);

        bool hovered = false;

        bool clicked = ImGui::ButtonBehavior({ { x - radius, std::min(y - radius, startY) }, { x + radius, std::max(y + radius, endY) } }, ImGui::GetID(set), &hovered, nullptr, ImGuiButtonFlags_PressedOnClick);

        if (clicked && set)
            *set = !*set;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        if (!set || !*set) {
            if (hovered) {
                draw_list->AddCircleFilled({ x, y }, 7.0f, IM_COL32(155, 55, 55, 55));
                draw_list->AddCircle({ x, y }, 7.0f, IM_COL32(155, 155, 155, 155));
            }
        } else {
            draw_list->AddCircleFilled({ x, y }, 7.0f, IM_COL32(255, 100, 100, 255));
        }

        return clicked;
    }
}
}
