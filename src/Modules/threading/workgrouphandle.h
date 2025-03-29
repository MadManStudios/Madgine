#pragma once

#if ENABLE_THREADING

#    include "workgroup.h"

namespace Engine {
namespace Threading {

    struct WorkGroupHandle {
        WorkGroupHandle() = default;
        WorkGroupHandle(const WorkGroupHandle &) = delete;

        template <typename F, typename... Args>
        WorkGroupHandle(std::string_view name, F &&main, Args &&... args)
        {
            std::promise<int> p;
            mResult = p.get_future();
            mThread = std::thread(&WorkGroupHandle::threadMain<F, Args...>, std::string { name }, std::move(p), std::forward<F>(main), std::forward<Args>(args)...);
        }

        ~WorkGroupHandle()
        {
            if (mThread.joinable()) {
                mThread.join();
            }
            if (mResult.valid())
                mResult.get();
        }

        WorkGroupHandle &operator=(WorkGroupHandle &&other)
        {
            assert(!mThread.joinable());
            mResult = std::move(other.mResult);
            mThread = std::move(other.mThread);
            return *this;
        }

        void detach()
        {
            assert(mThread.joinable());
            mThread.detach();
        }

    private:
        template <typename F, typename... Args>
        static void threadMain(std::string_view name, std::promise<int> p, std::remove_reference_t<F> main, std::remove_reference_t<Args>... args)
        {
            WorkGroup group { name };
            try {
                int result = TupleUnpacker::invokeDefaultResult(0, std::forward<F>(main), std::forward<Args>(args)...);
                p.set_value(result);
            } catch (std::exception &e) {
                LOG_ERROR("Uncaught Exception in Workgroup-MainThread!");
                LOG_ERROR(e.what());
                throw;
            }
        }

    private:
        std::future<int> mResult;
        std::thread mThread;
    };

}
}

#endif