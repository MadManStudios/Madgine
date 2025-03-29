#include "../toolslib.h"

#if ENABLE_MESSAGE_LOGGING

#include "messagelogger.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/serialize/streams/messagelogger.h"

#include "../imguiicons.h"

#include "Modules/threading/workgroup.h"

UNIQUECOMPONENT(Engine::Tools::MessageLogger);

METATABLE_BEGIN_BASE(Engine::Tools::MessageLogger, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::MessageLogger)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::MessageLogger, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::MessageLogger)

namespace Engine {
namespace Tools {

    #define IMGUI_ICON_IN "->|"
#define IMGUI_ICON_OUT "<-|"

    MessageLogger::MessageLogger(ImRoot &root)
        : Tool<MessageLogger>(root)
        , mLogger(Serialize::MessageLogger::getSingleton())
    {
    }

    MessageLogger::~MessageLogger()
    {
    }

    void MessageLogger::render()
    {
        if (ImGui::Begin("MessageLogger", &mVisible)) {
            for (const auto& [stream, data] : mLogger.mStreams) {
                if (!Threading::WorkGroup::self().contains(data.mThread))
                    continue;

                char buffer[256];
                sprintf(buffer, "%x", stream);
                ImGui::Checkbox(buffer, &mVisibleStreams[stream]);
                
                if (mVisibleStreams[stream]) {
                    if (ImGui::Begin(buffer, &mVisibleStreams[stream])) {
                        if (ImGui::TreeNode("Messages")) {
                            for (const Serialize::MessageLogger::MessageInfo &info : data.mMessages) {
                                if (ImGui::TreeNode(&info, "%s %d", info.mDir == Serialize::MessageLogger::INCOMING ? IMGUI_ICON_IN : IMGUI_ICON_OUT, info.mHeader.mMsgSize)) {
                                    ImGui::Text(std::string_view { info.mData.data(), info.mData.size() });
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::End();
                }
            }
        }
        ImGui::End();
    }

    std::string_view MessageLogger::key() const
    {
        return "MessageLogger";
    }

}
}

#endif