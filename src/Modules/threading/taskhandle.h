#pragma once

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT TaskHandle {

        TaskHandle() = default;
        TaskHandle(const TaskHandle &) = delete;
        TaskHandle(TaskHandle &&other);

        TaskHandle &operator=(const TaskHandle &) = delete;
        TaskHandle &operator=(TaskHandle &&);

        TaskHandle(CoroutineHandle<TaskPromiseBase> handle);
        template <typename T>
        TaskHandle(std::coroutine_handle<T> handle) noexcept
            : mHandle(std::coroutine_handle<TaskPromiseBase>::from_promise(handle.promise()))
        {
        }
        ~TaskHandle();

        void operator()();        
        void resume();

        std::coroutine_handle<TaskPromiseBase> release();

        TaskPromiseBase &promise();

        TaskQueue *queue() const;

        void *address() const;

        explicit operator bool() const;

    private:
        std::coroutine_handle<TaskPromiseBase> mHandle;
    };

}
}