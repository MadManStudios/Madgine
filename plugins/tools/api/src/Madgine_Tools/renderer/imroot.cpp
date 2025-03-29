#include "../toolslib.h"

#include "imroot.h"

#include "Modules/debug/profiler/profile.h"

#include "../toolbase.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "imgui/imguiaddons.h"

#include "im3d/im3d.h"

#include "Generic/container/safeiterator.h"

#include "Meta/serialize/operations.h"

#include "Meta/serialize/streams/serializestreamdata.h"

#include "Meta/serialize/hierarchy/statetransmissionflags.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/serialize/formats.h"

#include "Generic/projections.h"

#include "Interfaces/filesystem/fsapi.h"

METATABLE_BEGIN(Engine::Tools::ImRoot)
READONLY_PROPERTY(Tools, tools)
METATABLE_END(Engine::Tools::ImRoot)

namespace Engine {

namespace Tools {

    void *ToolReadOpen(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name) // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
    {
        ImRoot *root = static_cast<ImRoot *>(handler->UserData);

        root->finishToolRead();

        root->mToolReadBuffer.str("");
        for (ToolBase *tool : root->tools() | std::views::transform(projectionUniquePtrToPtr)) {
            if (tool->key() == name) {
                root->mToolReadTool = tool;
                return tool;
            }
        }
        return nullptr;
    }

    void ToolReadLine(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line) // Read: Called for every line of text within an ini entry
    {
        if (strlen(line) > 0) {
            static_cast<ImRoot *>(handler->UserData)->mToolReadBuffer << line << "\n";
        }
    }

    void ToolWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *out_buf) // Write: Output every entry into 'out_buf'
    {
        auto buf = std::make_unique<std::stringbuf>();
        std::stringbuf *outBuffer = buf.get();
        Serialize::FormattedSerializeStream out { Serialize::Formats::ini(), { std::move(buf) } };

        ImRoot *root = static_cast<ImRoot *>(handler->UserData);
        for (ToolBase *tool : root->tools() | std::views::transform(projectionUniquePtrToPtr)) {
            std::string name = std::string { tool->key() };
            out_buf->appendf("[Tool][%s]\n", name.c_str());

            Serialize::SerializableUnitPtr { tool }.writeState(out, nullptr, {}, true);
            out_buf->append(outBuffer->str().c_str());
            outBuffer->str("");

            out_buf->append("\n");
        }
    }

    ImRoot::ImRoot()
        : mCollector(*this)
    {
    }

    ImRoot::~ImRoot()
    {
    }

    Threading::Task<bool> ImRoot::init()
    {

        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "Tool";
        ini_handler.TypeHash = ImHashStr("Tool");
        ini_handler.ReadOpenFn = ToolReadOpen;
        ini_handler.ReadLineFn = ToolReadLine;
        ini_handler.WriteAllFn = ToolWriteAll;
        ini_handler.UserData = this;
        GImGui->SettingsHandlers.push_back(ini_handler);

        for (const std::unique_ptr<ToolBase> &tool : mCollector) {
            bool result = co_await tool->callInit();
            tool->setEnabled(result);
        }

        co_return true;
    }

    Threading::Task<void> ImRoot::finalize()
    {

        for (const std::unique_ptr<ToolBase> &tool : mCollector) {
            co_await tool->callFinalize();
        }
    }

    void ImRoot::render()
    {
        PROFILE_NAMED("ImGui - Rendering");

        ImGuiIO &io = ImGui::GetIO();

        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::NewFrame();
        Im3D::NewFrame();

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu("Project")) {
                ImGui::EndMenu();
            }

            for (ToolBase *tool : mCollector | std::views::transform(projectionUniquePtrToPtr)) {
                if (tool->isEnabled())
                    tool->renderMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                for (ToolBase *tool : mCollector | std::views::transform(projectionUniquePtrToPtr)) {
                    if (tool->isEnabled())
                        tool->renderStatus();
                }
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }

        finishToolRead();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("Madgine Root Window", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MadgineDockSpace");
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        mDockSpaceId = dockspace_id;

        for (ToolBase *tool : mCollector | std::views::transform(projectionUniquePtrToPtr)) {
            if (tool->isEnabled())
                tool->update();
        }

        mDialogContainer.render();

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::End();
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui::Render();
        else
            ImGui::EndFrame();

        ImGui::UpdatePlatformWindows();
    }

    const std::vector<std::unique_ptr<ToolBase>> &ImRoot::tools()
    {
        return mCollector;
    }

    ToolBase &ImRoot::getTool(size_t index)
    {
        return mCollector.get(index);
    }

    unsigned int ImRoot::dockSpaceId() const
    {
        return mDockSpaceId;
    }

    void ImRoot::finishToolRead()
    {
        if (mToolReadTool) {

            auto buf = std::make_unique<std::stringbuf>(mToolReadBuffer.str());
            Serialize::FormattedSerializeStream in { Serialize::Formats::ini(), { std::move(buf) } };

            Serialize::StreamResult result = Serialize::SerializableUnitPtr { mToolReadTool }.readState(in, nullptr, {}, true);            
            if (result.mState != Serialize::StreamState::OK) {
                LOG_ERROR(result);
            }

            mToolReadTool = nullptr;
        }
    }

    Dialog<Filesystem::Path> ImRoot::directoryPicker(Filesystem::Path path, Filesystem::Path selected)
    {
        DialogSettings &settings = co_await get_settings;
        settings.acceptText = "Open";

        do {            
            ImGui::DirectoryPicker(path, selected);
        } while (co_yield settings);
        co_return selected;
    }

    Dialog<Filesystem::Path> ImRoot::filePicker(bool allowNewFile, Filesystem::Path path, Filesystem::Path selected)
    {
        DialogSettings settings
        {
            .acceptText = allowNewFile ? "Save" : "Open",
            .declineText = "Cancel"
        };
        bool implicitlyAccepted = false;
        do {
            ImGui::FilePicker(path, selected, &implicitlyAccepted);
            settings.acceptPossible = !selected.empty() && (allowNewFile || Filesystem::exists(selected));
        } while (!implicitlyAccepted && (co_yield settings));
        co_return selected;
    }

    DialogContainer &ImRoot::dialogs()
    {
        return mDialogContainer;
    }

}
}
