#include "../toolslib.h"

#include "logviewer.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Interfaces/log/standardlog.h"

#include "Generic/coroutines/generator.h"

#include "Interfaces/filesystem/path.h"

#include "Modules/threading/workgroup.h"

#include "../imguiicons.h"

#include "../renderer/imroot.h"

#include "imgui/imgui_internal.h"

UNIQUECOMPONENT(Engine::Tools::LogViewer);

METATABLE_BEGIN_BASE(Engine::Tools::LogViewer, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::LogViewer)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::LogViewer, Engine::Tools::ToolBase)
FIELD(mMsgFilters)
SERIALIZETABLE_END(Engine::Tools::LogViewer)

namespace Engine {
namespace Tools {

    static void SeekCursorAndSetupPrevLine(float pos_y, float line_height)
    {
        // Set cursor position and a few other things so that SetScrollHereY() and Columns() can work when seeking cursor.
        // FIXME: It is problematic that we have to do that here, because custom/equivalent end-user code would stumble on the same issue.
        // The clipper should probably have a final step to display the last item in a regular manner, maybe with an opt-out flag for data sets which may have costly seek?
        ImGuiContext &g = *GImGui;
        ImGuiWindow *window = g.CurrentWindow;
        float off_y = pos_y - window->DC.CursorPos.y;
        window->DC.CursorPos.y = pos_y;
        window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, pos_y - g.Style.ItemSpacing.y);
        window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height; // Setting those fields so that SetScrollHereY() can properly function after the end of our clipper usage.
        window->DC.PrevLineSize.y = (line_height - g.Style.ItemSpacing.y); // If we end up needing more accurate data (to e.g. use SameLine) we may as well make the clipper have a fourth step to let user process and display the last item in their list.
        if (ImGuiOldColumns *columns = window->DC.CurrentColumns)
            columns->LineMinY = window->DC.CursorPos.y; // Setting this so that cell Y position are set properly
        if (ImGuiTable *table = g.CurrentTable) {
            table->RowPosY2 = window->DC.CursorPos.y;
            if (table->IsInsideRow)
                ImGui::TableEndRow(table);
            const int row_increase = (int)((off_y / line_height) + 0.5f);
            //table->CurrentRow += row_increase; // Can't do without fixing TableEndRow()
            table->RowBgColorCounter += row_increase;
        }
    }

    static constexpr size_t sLookupStep = 1;
    static constexpr std::array<const char *, 5> sIcons { "D", IMGUI_ICON_INFO, IMGUI_ICON_WARNING, IMGUI_ICON_ERROR, "F" };
    static constexpr std::array<ImColor, 5> sColors { { { 0.0f, 0.78f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.78f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } };

    LogViewer::LogViewer(ImRoot &root)
        : Tool<LogViewer>(root, true)
        , mWorkgroup(&Threading::WorkGroup::self())
    {
        for (Log::MessageType type : Log::MessageType::values()) {
            mMsgCounts[type] = 0;
            mMsgFilters[type] = true;
        }
        Log::Log::addListener(this);
    }

    LogViewer::~LogViewer()
    {
        Log::Log::removeListener(this);
    }

    void LogViewer::render()
    {
        if (ImGui::Begin("LogViewer", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Down, 0.3f, true, ImGuiCond_FirstUseEver);

            int mTotalMsgCount = 0;

            for (Log::MessageType type : Log::MessageType::values()) {
                ImGui::PushStyleColor(ImGuiCol_Text, sColors[type].Value);
                mIsDirty |= ImGui::Checkbox(sIcons[type], &mMsgFilters[type]);
                ImGui::PopStyleColor();
                ImGui::SameLine();

                if (mMsgFilters[type])
                    mTotalMsgCount += mMsgCounts[type];
            }
            mIsDirty |= ImGui::InputText("Filter", &mMessageWordFilter);

            std::lock_guard guard { mMutex };

            if (ImGui::BeginTable("Messages", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit)) {

                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 12.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Log", ImGuiTableColumnFlags_WidthStretch, 0.1f);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.3f);                
                ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
                float messageWidth = ImGui::GetContentRegionAvail().x;
                if (messageWidth != mCachedWidth) {
                    mCachedWidth = messageWidth;
                    mIsDirty = true;
                }

                if (mIsDirty) {
                    mIsDirty = false;
                    mFilteredMsgCount = 0;
                    mFilteredOffsetAcc = 0.0f;
                    mLookup.clear();
                    size_t i = 0;
                    for (LogEntry &entry : mEntries) {
                        if (filter(entry))
                            addFilteredMessage(i, entry.mMsg);
                        ++i;
                    }
                }


                ImGuiWindow *window = ImGui::GetCurrentWindow();
                float lossyness = window->DC.CursorStartPosLossyness.y;

                float startPos = window->DC.CursorPos.y;

                float startOffset = window->ClipRect.Min.y - startPos - lossyness;
                float endOffset = window->ClipRect.Max.y - startPos - lossyness;

                auto begin = std::ranges::find_if(
                    std::views::reverse(mLookup), [&](float off) { return off <= startOffset; }, &Lookup::mOffset)
                                 .base();
                if (begin != mLookup.begin())
                    --begin;

                auto end = std::ranges::find_if(
                    mLookup, [&](float off) { return off >= endOffset; }, &Lookup::mOffset);

                if (begin != mLookup.end()) {

                    const Lookup &lookup = *begin;
                    size_t count = (end - begin) * sLookupStep;

                    auto it = mEntries.begin() + lookup.mIndex;
                    SeekCursorAndSetupPrevLine(startPos + lookup.mOffset + lossyness, calculateTextHeight(it->mMsg));

                    for (; it != mEntries.end() && count > 0; ++it) {
                        const LogEntry &entry = *it;
                        if (filter(entry)) {
                            /* if (skip > 0) {
                                --skip;
                            } else */
                            {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::TextColored(sColors[entry.mType], "%s", sIcons[entry.mType]);
                                ImGui::TableNextColumn();
                                ImGui::TextWrapped("%s", entry.mMsg.c_str());
                                ImGui::TableNextColumn();
                                ImGui::TextWrapped("%s", entry.mLog->getName().c_str());
                                ImGui::TableNextColumn();
                                if (entry.mFile) {
                                    Filesystem::Path file = entry.mFile;
                                    ImGui::TextWrapped("%s", file.c_str());
                                }
                                ImGui::TableNextColumn();
                                if (entry.mFile)
                                    ImGui::Text("%zu", entry.mLine);
                                --count;
                            }
                        }
                    }
                }

                SeekCursorAndSetupPrevLine(startPos + mFilteredOffsetAcc + lossyness, 0.0f);

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void LogViewer::renderStatus()
    {
        for (Log::MessageType type : Log::MessageType::values()) {
            if (mMsgCounts[type] > 0) {
                ImGui::TextColored(sColors[type], "%s %d", sIcons[type], static_cast<int>(mMsgCounts[type]));
            }
        }
        ImGui::Separator();
    }

    void LogViewer::messageLogged(std::string_view message, Log::MessageType lml, const char *file, size_t line, Log::Log *log)
    {
        if (Threading::WorkGroup::isInitialized() && mWorkgroup != &Threading::WorkGroup::self())
            return;
        ++mMsgCounts[lml];
        std::lock_guard guard { mMutex };
        if (filter(mEntries.emplace_back(std::string { message }, lml, file, line, log)))
            addFilteredMessage(mEntries.size() - 1, message);
    }

    std::string_view LogViewer::key() const
    {
        return "LogViewer";
    }

    bool LogViewer::filter(const LogEntry &entry)
    {
        return mMsgFilters[entry.mType] && (mMessageWordFilter.empty() || StringUtil::contains(entry.mMsg, mMessageWordFilter));
    }

    void LogViewer::addFilteredMessage(size_t index, std::string_view text)
    {
        if (mFilteredMsgCount % sLookupStep == 0) {
            mLookup.push_back({ index, mFilteredOffsetAcc });
        }
        ++mFilteredMsgCount;
        if (Threading::WorkGroup::isInitialized() && ImGui::GetCurrentContext() && ImGui::GetCurrentContext()->Font)
            mFilteredOffsetAcc += calculateTextHeight(text);
        else
            mIsDirty = true;
    }

    float LogViewer::calculateTextHeight(std::string_view text)
    {

        return ImGui::CalcTextSize(text.data(), text.data() + text.size(), false, mCachedWidth).y + ImGui::GetCurrentContext()->Style.ItemSpacing.y;
    }

}
}
