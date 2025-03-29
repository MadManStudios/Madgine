#include "../interfaceslib.h"

#include "fsapi.h"
#include "filequery.h"

namespace Engine {
namespace Filesystem {

    FileQueryHandle::FileQueryHandle(FileQueryHandle &&other)
        : mPath(std::move(other.mPath))
        , mHandle(std::exchange(other.mHandle, nullptr))
    {
    }

    FileQueryHandle::~FileQueryHandle()
    {
        close();
    }

    const Path &FileQueryHandle::path() const
    {
        return mPath;
    }

    FileQueryHandle::operator bool()
    {
        return mHandle != nullptr;
    }

    bool FileQueryHandle::operator==(const FileQueryHandle &other) const
    {
        return mHandle == other.mHandle && mPath == other.mPath;
    }

    FileQueryIterator::FileQueryIterator(const FileQuery *query)
        : mQuery(query)
        , mBuffer(createQueryState())
    {
        enterDir();
    }

    /* FileQueryIterator::FileQueryIterator(FileQueryIterator &&other)
        : mHandles(std::move(other.mHandles))
        , mQuery(other.mQuery)
        , mBuffer(std::move(other.mBuffer))
    {
    }*/

    FileQueryIterator::~FileQueryIterator()
    {
    }

    FileQueryResult FileQueryIterator::operator*() const
    {
        return { &mHandles.back(), mBuffer.get() };
    }

    FileQueryIterator &FileQueryIterator::operator++()
    {
        if (mHandles.back().advance(*mBuffer))
            verify();
        else
            leaveDir();
        return *this;
    }

    void FileQueryIterator::operator++(int)
    {
        ++(*this);
    }

    bool FileQueryIterator::operator!=(const FileQueryIterator &other) const
    {
        assert(mQuery == other.mQuery);
        return mHandles != other.mHandles;
    }

    bool FileQueryIterator::operator==(const FileQuerySentinel &sen) const
    {
        assert(mQuery == sen.mQuery);
        return mHandles.empty();
    }

    bool operator==(const FileQuerySentinel &sen, const FileQueryIterator &self)
    {
        assert(self.mQuery == sen.mQuery);
        return self.mHandles.empty();
    }

    bool FileQueryIterator::operator!=(const FileQuerySentinel &sen) const
    {
        assert(mQuery == sen.mQuery);
        return !mHandles.empty();
    }

    void FileQueryIterator::enterDir()
    {
        Path p = mHandles.empty() ? mQuery->mPath : **this;

        FileQueryHandle handle(p, *mBuffer);
        if (handle) {
            mHandles.emplace_back(std::move(handle));
            verify();
        }
    }

    void FileQueryIterator::leaveDir()
    {
        mHandles.pop_back();
        if (!mHandles.empty())
            ++(*this);
    }

    void FileQueryIterator::verify()
    {
        if (!mHandles.empty()) {
            if (currentIsDir()) {
                if (mQuery->mRecursive) {
                    enterDir();
                } else if (!mQuery->mShowDirs) {
                    ++(*this);
                }
            } else {
                if (!mQuery->mShowFiles) {
                    ++(*this);
                }
			}
        }
    }

    bool FileQueryIterator::currentIsDir()
    {
        return isDir(*mBuffer);
    }

    FileQueryIterator FileQuery::begin() const
    {
        return { this };
    }

    FileQuerySentinel FileQuery::end() const
    {
        return { this };
    }

    bool FileQueryResult::isDir() const
    {
        return Filesystem::isDir(*mState);
    }

    FileQueryResult::operator Path() const
    {
        return path();
    }

    Path FileQueryResult::path() const
    {
        return mHandle->path() / filename(*mState);
    }


}
}