#pragma once

#include "Madgine/debug/debugger.h"
#include "Madgine/debug/debuglistener.h"

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

struct ImVec2;
struct ImDrawList;

namespace Engine {
namespace Tools {

    MADGINE_DEBUGGER_TOOLS_EXPORT bool BeginDebuggablePanel(const char *name);
    MADGINE_DEBUGGER_TOOLS_EXPORT void EndDebuggablePanel();
    MADGINE_DEBUGGER_TOOLS_EXPORT void DrawDebugMarker(float y);
    MADGINE_DEBUGGER_TOOLS_EXPORT bool Breakpoint(float startY, float endY, bool *set);

    struct MADGINE_DEBUGGER_TOOLS_EXPORT DebuggerView : Tool<DebuggerView>, Debug::DebugListener {

        SERIALIZABLEUNIT(DebuggerView)

        enum class ControlButton {
            NONE,
            PLAY,
            STEP,
            PAUSE,
            STOP
        };

        DebuggerView(ImRoot &root);
        DebuggerView(const DebuggerView &) = delete;

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void render() override;
        virtual void renderMenu() override;

        void renderDebugContext(const Debug::ContextInfo &context);
        void renderLifetime(Debug::DebuggableLifetimeBase &lifetime);

        void setCurrentContext(Debug::ContextInfo &context);

        void onSuspend(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type) override;
        bool wantsPause(Debug::ContextInfo &context, TypedPtr location, Debug::ContinuationType type, IndexType<size_t> line) override;

        std::string_view key() const override;

        template <auto Visualizer>
        void registerDebugLocationVisualizer()
        {
            using T = std::remove_reference_t<typename CallableTraits<decltype(Visualizer)>::argument_types::template select<2>>;
            auto [it, b] = mDebugLocationVisualizers.try_emplace(typeid(std::remove_pointer_t<T>), [](DebuggerView &view, const Debug::ContextInfo &context, const void *location, TypedPtr inlineLocation) -> std::vector<TypedPtr> {
                T typedLocation = static_cast<T>(location);
                return Visualizer(view, context, typedLocation, inlineLocation);
            });
            assert(b);
        }

        std::vector<TypedPtr> visualizeDebugLocation(const Debug::ContextInfo &context, TypedPtr location, TypedPtr inlineLocation);

        ControlButton contextControls(bool running);

    private:
        Debug::Debugger &mDebugger;
        Debug::ContextInfo *mSelectedContext = nullptr;
        Debug::DebugLocation *mSelectedLocation = nullptr;

        std::map<std::type_index, std::vector<TypedPtr> (*)(DebuggerView &, const Debug::ContextInfo &, const void *, TypedPtr), std::less<>> mDebugLocationVisualizers;

        Inspector *mInspector = nullptr;
    };

}
}