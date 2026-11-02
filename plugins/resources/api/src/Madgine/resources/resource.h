#pragma once

namespace Engine {
namespace Resources {

    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<bool> queueLoad(Threading::Task<bool> task, Threading::TaskQueue &queue);
    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<void> queueUnload(Threading::Task<void> task, Threading::TaskQueue &queue);

    template <typename Interface>
    struct ResourceDataInfoBase {
        ResourceDataInfoBase(typename Interface::Resource *res)
            : mResource(res)
        {
        }

        typename Interface::Resource *resource() const
        {
            return mResource;
        }

    private:
        typename Interface::Resource *mResource;
    };

    template <typename Interface>
    struct ResourceDataInfo : ResourceDataInfoBase<Interface> {
        using ResourceDataInfoBase<Interface>::ResourceDataInfoBase;
    };

    struct ResourceControl {
        template <typename Ctor, typename Loader>
        ResourceControl(Ctor &&ctor, Loader *loader, auto &data, const auto &info, Platform::Filesystem::FileEventType event)
            : mLoadingTask(queueLoad(ctor(loader, data, info, event), loader->loadingTaskQueue()))
        {
        }

        void incRef()
        {
            ++mRefCount;
        }

        bool decRef()
        {
            uint32_t oldCount = mRefCount--;
            assert(oldCount > 0);
            return oldCount > 1;
        }

        void setPersistent(bool b)
        {
            bool previous = mPersistent.exchange(b);
            if (previous != b) {
                if (b) {
                    incRef();
                } else {
                    bool alive = decRef();
                    assert(alive);
                }
            }
        }

        bool verify() const
        {
            assert(mLoadingTask.valid());
            return mLoadingTask.is_ready() && mLoadingTask;
        }

        const Threading::TaskFuture<bool> &loadingTask() const
        {
            return mLoadingTask;
        }

        std::atomic<uint32_t> mRefCount = 0;

        std::atomic<bool> mPersistent = false;
        const Threading::TaskFuture<bool> mLoadingTask;
    };

    template <typename Loader>
    struct ResourceData {

        template <typename Ctor>
        ResourceData(Ctor &&ctor, Loader *loader, typename Loader::Resource *res, Platform::Filesystem::FileEventType event)
            : mInfo(res)
            , mControl(std::forward<Ctor>(ctor), loader, mData, mInfo, event)
        {
        }

        typename Loader::Data *verified(bool verified)
        {
            if (verified && !mControl.verify())
                return nullptr;
            return std::addressof(mData);
        }

        typename Loader::Data mData;
        typename Loader::ResourceDataInfo mInfo;
        ResourceControl mControl;
        typename Containers::container_traits<typename Loader::DataContainer>::position_handle mHolder;
    };

    template <typename Loader>
    struct Resource : Loader::Interface::Resource {

        Resource(const std::string &name, const Platform::Filesystem::Path &path = {}, typename Loader::Ctor ctor = {})
            : Loader::Interface::Resource(name, path)
            , mCtor(ctor ? std::move(ctor) : Loader::Interface::template toCtor<Loader>(&Loader::loadImpl))
        {
        }

        typename Loader::Ctor mCtor;
    };

}
}