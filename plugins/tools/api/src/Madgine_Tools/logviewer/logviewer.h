#pragma once

#include "Platform/log/loglistener.h"

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct LogViewer : Tool<LogViewer>, Platform::Log::LogListener {
        SERIALIZABLEUNIT(LogViewer)

        LogViewer(ImRoot &root);
        ~LogViewer();

        virtual void render() override;

        virtual void messageLogged(EMSCRIPTEN_WORKAROUND(std::string_view) message, Platform::Log::MessageType lml, const char *file, size_t line, Platform::Log::Log *log) override;

        std::string_view key() const override;

    protected:
        struct LogEntry {
            LogEntry(std::string msg, Platform::Log::MessageType type, const char *file, size_t line, Platform::Log::Log *log)
                : mMsg(msg)
                , mType(type)
                , mFile(file)
                , mLine(line)
                , mLog(log)
            {
            }

            std::string mMsg;
            Platform::Log::MessageType mType;
            const char *mFile;
            size_t mLine;
            Platform::Log::Log *mLog;
        };

        bool filter(const LogEntry &entry);

        void addFilteredMessage(size_t index, const LogEntry &entry);

        float calculateTextHeight(const LogEntry &entry);

    private:
        Threading::WorkGroup *mWorkgroup;
        std::deque<LogEntry> mEntries;
        std::array<size_t, Platform::Log::MessageType::COUNT> mMsgCounts;
        std::mutex mMutex;

        std::array<bool, Platform::Log::MessageType::COUNT> mMsgFilters;
        std::string mMessageWordFilter;
        size_t mFilteredMsgCount = 0;
        struct Lookup {
            size_t mIndex;
            float mOffset;
        };
        std::vector<Lookup> mLookup;
        float mFilteredOffsetAcc = 0.0f;
        bool mIsDirty = true;
        float mCachedWidth[3] = {};
    };

}
}