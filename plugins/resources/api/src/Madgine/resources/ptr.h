#pragma once

namespace Engine {
namespace Resources {

    template <typename Loader, typename Data>
    struct Ptr {

        Ptr() = default;
        template <typename Data2>
        Ptr(std::unique_ptr<Data2> data)
            : mData(std::move(data))
        {
        }
        Ptr(Ptr &&) = default;

        template <typename Loader2, typename Data2>
        Ptr(Ptr<Loader2, Data2> &&other)
            : mData(std::move(other.mData))
        {
            static_assert(std::derived_from<Loader2, Loader> || std::derived_from<Loader, Loader2>, "Invalid conversion-type for Ptr!");
        }

        ~Ptr()
        {
            reset();
        }

        Ptr &operator=(Ptr &&other)
        {
            std::swap(mData, other.mData);
            return *this;
        }

        operator Data *() const
        {
            return mData.get();
        }

        Data *operator->() const
        {
            return mData.get();
        }

        template <typename C>
        Threading::TaskFuture<bool> create(C &&c, Loader *loader = &Loader::getSingleton())
        {
            return Loader::loadUnnamed(*this, std::forward<C>(c), loader);
        }

        template <typename C>
        Threading::Task<bool> createTask(C &&c, Loader *loader = &Loader::getSingleton())
        {
            return Loader::loadUnnamedTask(*this, std::forward<C>(c), loader);
        }

        void reset()
        {
            if (mData) {
                Loader::unload(std::move(mData));
                mData.reset();
            }
        }

        std::unique_ptr<Data> mData;
    };

}
}