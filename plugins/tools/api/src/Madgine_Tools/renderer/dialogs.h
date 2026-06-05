#pragma once

#include "Generic/closure.h"
#include "Generic/enum.h"
#include "Generic/execution/concepts.h"
#include "Generic/type_pack.h"

namespace Engine {
namespace Tools {

    ENUM(DialogResult,
        Accepted,
        Declined,
        Canceled)

    struct MADGINE_TOOLS_EXPORT DialogSettings {

        DialogSettings() = default;

        DialogSettings(const DialogSettings &) = delete;

        DialogSettings &operator=(const DialogSettings &) = delete;

        bool showAccept = true;
        bool showDecline = true;
        bool showCancel = false;
        std::string acceptText = "Yes";
        std::string declineText = "No";
        std::string cancelText = "Cancel";
        bool acceptPossible = true;

        bool allowApplyToAll = false;
        bool callbackOnDecline = false;

        std::string header = " ";

        void accept();
        void acceptAll();
        void decline();
        void declineAll();
        void cancel();

        bool completed() const;
        bool accepted() const;
        bool declined() const;
        bool cancelled() const;

        template <typename... T>
        void open(Dialog<T...> dialog, T &...out)
        {
            dialog.setCallback([&](const T &...result) {
                (DefaultAssign {}(out, result), ...);
            });
            open(std::move(dialog.mHandle));
        }

        void open(Execution::CoroutineHandle<DialogPromise> handle);

        void setParent(DialogSettings &parent);

        
        DialogSettings &root();
        const DialogSettings &root() const;

        DialogSettings *mParent = nullptr;

        std::optional<DialogResult> mResult;
        std::vector<Execution::CoroutineHandle<DialogPromise>> mSubDialogs;

        size_t mModalLayers = 0;
    };

    struct DialogDeclined { };
    constexpr DialogDeclined dialogDeclined;

    struct MADGINE_TOOLS_EXPORT DialogContainer {
        DialogContainer() = default;
        DialogContainer(const DialogContainer &) = delete;

        DialogContainer &operator=(const DialogContainer &) = delete;

        void show(Execution::CoroutineHandle<DialogPromise> dialog);

        template <typename Dialog, typename F>
        void show(Dialog dialog, F &&f)
        {
            dialog.setCallback(std::forward<F>(f));
            show(std::move(dialog.mHandle));
        }

        void showGrouped(std::string_view name, Execution::CoroutineHandle<DialogPromise> dialog);

        template <typename Dialog, typename F>
        void showGrouped(std::string_view name, Dialog dialog, F &&f)
        {
            dialog.setCallback(std::forward<F>(f));
            showGrouped(name, std::move(dialog.mHandle));
        }

        void render();

    protected:
        bool renderHeader(DialogSettings &settings);
        void renderFooter(DialogSettings &settings);

        void handleDialogs(std::vector<Execution::CoroutineHandle<DialogPromise>> &dialogs);

        struct DialogGroup {
            DialogGroup(DialogContainer &);

            void render();
            void addDialog(Execution::CoroutineHandle<DialogPromise> dialog);

            DialogContainer &mContainer;
            DialogSettings mSettings;     
            std::deque<std::vector<Execution::CoroutineHandle<DialogPromise>>> mDialogs;
        };

    private:
        std::vector<Execution::CoroutineHandle<DialogPromise>> mDialogs;

        std::map<std::string, DialogGroup> mDialogGroups;
    };

    template <typename... T>
    struct Dialog;
    template <typename... T>
    struct AwaitableDialog;

    struct DialogPromise {
        DialogPromise()
        {
        }

        ~DialogPromise()
        {
            assert(!mContainer);
        }

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        struct YieldSuspender {
            bool await_ready() const noexcept
            {
                return mPromise.mSettings.completed();
            }

            void await_suspend(Execution::CoroutineHandle<DialogPromise> self) const noexcept
            {
                assert(mPromise.mContainer && &self.promise() == &mPromise);
                *mPromise.mContainer = std::move(self);
            }
            bool await_resume() const noexcept { return !mPromise.mSettings.completed(); }

            DialogPromise &mPromise;
        };

        struct FinalAwaiter {

            bool await_ready() const noexcept
            {
                return false;
            }

            std::coroutine_handle<> await_suspend(Execution::CoroutineHandle<DialogPromise> self) noexcept
            {
                if (mResumer) {
                    std::swap(self->mContainer, mResumer.promise().mContainer);
                    return mResumer.release();
                } else {
                    *self->mContainer = std::move(self);
                    return std::noop_coroutine();
                }
            }
            void await_resume() const noexcept
            {
                std::terminate();
            }

            Execution::CoroutineHandle<DialogPromise> mResumer;
        };

        FinalAwaiter final_suspend() noexcept
        {
            assert(mSettings.completed());
            return { std::move(mResumer) };
        }

        YieldSuspender yield_value(DialogSettings &settings)
        {
            assert(&settings == &mSettings);
            return { *this };
        }

        void unhandled_exception()
        {
            throw;
        }

        void suspend(Execution::CoroutineHandle<DialogPromise> resumer)
        {
            std::swap(resumer.promise().mContainer, mContainer);
            mSettings.setParent(resumer.promise().mSettings);
            mResumer = std::move(resumer);
        }

        DialogSettings mSettings;
        Execution::CoroutineHandle<DialogPromise> mResumer;
        Execution::CoroutineHandle<DialogPromise> *mContainer = nullptr;
    };

    struct get_dialog_settings_t {
    };

    constexpr get_dialog_settings_t get_dialog_settings;

    struct get_dialog_settings_helper_t {
        constexpr bool await_ready() const noexcept
        {
            return true;
        }
        void await_suspend(std::coroutine_handle<>) const noexcept
        {
            std::terminate();
        }
        constexpr DialogSettings &await_resume() const noexcept
        {
            return mPromise.mSettings;
        }

        DialogPromise &mPromise;
    };

    template <typename... T>
    struct AwaitableDialog {
        AwaitableDialog(Dialog<T...> dialog)
            : mDialog(std::move(dialog))
        {
        }

        constexpr bool await_ready() const noexcept
        {
            return false;
        }

        std::coroutine_handle<> await_suspend(Execution::CoroutineHandle<DialogPromise> handle) noexcept
        {
            mDialog.mHandle.promise().suspend(std::move(handle));
            mDialog.setCallback([this](T... result) { mResult.emplace(result...); });
            return mDialog.mHandle.release();
        }

        std::optional<std::tuple<T...>> await_resume() const noexcept
        {
            return std::move(mResult);
        }

        Dialog<T...> mDialog;
        std::optional<std::tuple<T...>> mResult;
    };

    template <typename... T>
    struct Dialog {
        struct promise_type : DialogPromise {

            Dialog<T...> get_return_object()
            {
                return { Execution::CoroutineHandle<promise_type>::fromPromise(*this) };
            }

            void return_value(std::tuple<T...> value)
            {
                if (!mSettings.completed())
                    mSettings.accept();
                if (mCallback && (mSettings.accepted() || (mSettings.callbackOnDecline && mSettings.declined())))
                    TupleUnpacker::invokeFromTuple(mCallback, std::move(value));
            }

            template <typename A>
            decltype(auto) await_transform(A &&a)
            {
                if constexpr (std::same_as<A, const get_dialog_settings_t &>) {
                    return get_dialog_settings_helper_t { *this };
                } else if constexpr (Concepts::InstanceOf<A, Dialog>) {
                    return AwaitableDialog(std::forward<A>(a));
                } else {
                    return std::forward<A>(a);
                }
            }

            Closure<void(T...)> mCallback;
        };

        template <typename F>
        void setCallback(F &&f)
        {
            assert(!mHandle->mCallback);
            mHandle->mCallback = std::forward<F>(f);
        }

        Execution::CoroutineHandle<promise_type> mHandle;
    };

}
}
