#pragma once

namespace Engine {
namespace Resources {

    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<bool> queueLoad(Threading::Task<bool> task, Threading::TaskQueue *queue);
    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<void> queueUnload(Threading::Task<void> task, Threading::TaskQueue *queue);

    template <typename Interface>
    struct ResourceDataInfoBase {
        ResourceDataInfoBase(typename Interface::Resource *res)
            : mResource(res)
        {
        }

        ResourceDataInfoBase(ResourceDataInfoBase &&other) noexcept
            : mResource(other.mResource)
            , mRefCount(other.mRefCount.exchange(0))
        {
        }

        ResourceDataInfoBase &operator=(ResourceDataInfoBase &&other) noexcept
        {
            mResource = other.mResource;
            mRefCount = other.mRefCount.exchange(0);
            return *this;
        }

        typename Interface::Resource *resource() const
        {
            return mResource;
        }

        void incRef()
        {
            ++mRefCount;
        }

        bool decRef()
        {
            uint32_t oldCount = mRefCount--;
            assert(oldCount > 0);
            return oldCount > 1 || mPersistent;
        }

        void setPersistent(bool b)
        {
            mPersistent = b;
        }

        Threading::TaskFuture<bool> setLoadingTask(Threading::Task<bool> task, Threading::TaskQueue *queue)
        {
            std::unique_lock lock { mMutex };

            if (mLoadingTask.valid()) {
                mLoadingTask = queueLoad(mLoadingTask.then_task(std::move(task)), queue);
                assert(!mUnloadingTask.valid());
            } else if (mUnloadingTask.valid()) {
                mLoadingTask = queueLoad(mUnloadingTask.then_task(std::move(task)), queue);
                mUnloadingTask.reset();
            } else {
                mLoadingTask = queueLoad(std::move(task), queue);
            }
            return mLoadingTask;
        }

        Threading::TaskFuture<void> setUnloadingTask(Threading::Task<void> task, Threading::TaskQueue *queue)
        {
            std::unique_lock lock { mMutex };
            if (mLoadingTask.valid()) {
                mUnloadingTask = queueUnload(mLoadingTask.then_task(std::move(task)), queue);
                mLoadingTask.reset();
            }
            return mUnloadingTask.valid() ? mUnloadingTask : Threading::TaskFuture<void>::make_ready();
        }

        bool verify() const
        {
            std::unique_lock lock { mMutex };
            return mLoadingTask.valid() && mLoadingTask.is_ready() && mLoadingTask && !mUnloadingTask.valid();
        }

        Threading::TaskFuture<bool> loadingTask()
        {
            std::unique_lock lock { mMutex };
            return mLoadingTask;
        }

        Threading::TaskFuture<void> unloadingTask()
        {
            std::unique_lock lock { mMutex };
            return mUnloadingTask;
        }

    private:
        typename Interface::Resource *mResource;
        std::atomic<uint32_t> mRefCount = 0;
        bool mPersistent = false;
        Threading::TaskFuture<bool> mLoadingTask;
        Threading::TaskFuture<void> mUnloadingTask;
        mutable std::mutex mMutex;
    };

    template <typename Interface>
    struct ResourceDataInfo : ResourceDataInfoBase<Interface> {
        using ResourceDataInfoBase<Interface>::ResourceDataInfoBase;
    };

    template <typename Loader>
    struct ResourceData {

        ResourceData(typename Loader::Resource *res)
            : mInfo(res)
        {
        }

        typename Loader::Data *verified(bool verified)
        {
            if (verified && !mInfo.verify())
                return nullptr;
            return std::addressof(mData);
        }

        typename Loader::ResourceDataInfo mInfo;
        typename Loader::Data mData;
        typename container_traits<typename Loader::DataContainer>::position_handle mHolder;
    };

    template <typename Loader>
    struct Resource : Loader::Interface::Resource {

        Resource(const std::string &name, const Filesystem::Path &path = {}, typename Loader::Ctor ctor = {})
            : Loader::Interface::Resource(name, path)
            , mCtor(ctor ? std::move(ctor) : Loader::Interface::template toCtor<Loader>(&Loader::loadImpl))
        {
        }

        typename Loader::Handle loadData()
        {
            return Loader::load(this);
        }

        Threading::TaskFuture<void> forceUnload()
        {
            return Loader::unload(this);
        }

        typename Loader::Ctor mCtor;
    };

}
}