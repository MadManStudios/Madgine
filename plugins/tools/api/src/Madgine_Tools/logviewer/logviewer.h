#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

#include "Interfaces/log/loglistener.h"

namespace Engine {
namespace Tools {

    struct LogViewer : Tool<LogViewer>, Log::LogListener {
        SERIALIZABLEUNIT(LogViewer)

        LogViewer(ImRoot &root);
        ~LogViewer();

        virtual void render() override;
        virtual void renderStatus() override;

        virtual void messageLogged(std::string_view message, Log::MessageType lml, const char *file, size_t line, Log::Log *log) override;

        std::string_view key() const override;

    protected:
        struct LogEntry {
            LogEntry(std::string msg, Log::MessageType type, const char *file, size_t line, Log::Log *log)
                : mMsg(msg)
                , mType(type)
                , mFile(file)
                , mLine(line)
                , mLog(log)
            {
            }

            std::string mMsg;
            Log::MessageType mType;
            const char *mFile;
            size_t mLine;
            Log::Log *mLog;
        };

        bool filter(const LogEntry &entry);

        void addFilteredMessage(size_t index, std::string_view text);

        float calculateTextHeight(std::string_view text);

    private:
        Threading::WorkGroup *mWorkgroup;
        std::deque<LogEntry> mEntries;
        std::array<size_t, Log::MessageType::COUNT> mMsgCounts;
        std::mutex mMutex;

        std::array<bool, Log::MessageType::COUNT> mMsgFilters;
        std::string mMessageWordFilter;
        size_t mFilteredMsgCount = 0;
        struct Lookup {
            size_t mIndex;
            float mOffset;
        };
        std::vector<Lookup> mLookup;
        float mFilteredOffsetAcc = 0.0f;
        bool mIsDirty = true;
        float mCachedWidth = 0.0f;
    };

}
}

REGISTER_TYPE(Engine::Tools::LogViewer)