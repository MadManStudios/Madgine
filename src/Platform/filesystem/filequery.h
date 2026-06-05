#pragma once

#include "path.h"

namespace Engine {
namespace Platform {
    namespace Filesystem {

        FileQueryState *createQueryState();
        PLATFORM_EXPORT void destroyQueryState(FileQueryState *state);
        struct FileQueryStateDeleter {
            void operator()(FileQueryState *state) { destroyQueryState(state); }
        };

        PLATFORM_EXPORT bool isDir(const FileQueryState &data);
        PLATFORM_EXPORT const char *filename(const FileQueryState &data);

        struct PLATFORM_EXPORT FileQueryHandle {
            FileQueryHandle(Path p, FileQueryState &data);
            FileQueryHandle(FileQueryHandle &&other);
            ~FileQueryHandle();

            bool advance(FileQueryState &data);
            explicit operator bool();
            bool operator==(const FileQueryHandle &other) const;

            const Path &path() const;

            void close();

        private:
            Path mPath;
            void *mHandle;
        };

        struct PLATFORM_EXPORT FileQueryResult {
            const FileQueryHandle *mHandle;
            const FileQueryState *mState;

            bool isDir() const;
            operator Path() const;
            Path path() const;
        };

        struct PLATFORM_EXPORT FileQuerySentinel {

            const FileQuery *mQuery;
        };

        struct PLATFORM_EXPORT FileQueryIterator {

            using difference_type = int;
            using value_type = FileQueryResult;

            FileQueryIterator(const FileQuery *query);
            FileQueryIterator(FileQueryIterator &&) = default;
            ~FileQueryIterator();

            FileQueryIterator &operator=(FileQueryIterator &&) = default;

            FileQueryResult operator*() const;
            FileQueryIterator &operator++();
            void operator++(int);

            bool operator!=(const FileQueryIterator &other) const;
            bool operator==(const FileQuerySentinel &sen) const;
            friend bool operator==(const FileQuerySentinel &sen, const FileQueryIterator &self);
            bool operator!=(const FileQuerySentinel &sen) const;

            void enterDir();
            void leaveDir();

            bool currentIsDir();

        protected:
            void verify();

            void increment();

        private:
            std::vector<FileQueryHandle> mHandles;
            const FileQuery *mQuery;
            std::unique_ptr<FileQueryState, FileQueryStateDeleter> mBuffer;
        };

        struct PLATFORM_EXPORT FileQuery {

            FileQueryIterator begin() const;
            FileQuerySentinel end() const;

            Path mPath;
            bool mRecursive = false;
            bool mShowDirs = false;
            bool mShowFiles = true;
        };

    }
}
}