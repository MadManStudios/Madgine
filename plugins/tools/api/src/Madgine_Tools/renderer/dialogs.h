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

        std::string header = " ";

        void accept();
        void decline();
        void cancel();

        template <typename... T>
        void open(Dialog<T...> dialog, T &...out)
        {
            dialog.setCallback([&](const T &...result) {
                (DefaultAssign {}(out, result), ...);
            });
            open(std::move(dialog.mHandle));
        }

        void open(CoroutineHandle<DialogPromise> handle);

        std::optional<DialogResult> result;
        std::vector<CoroutineHandle<DialogPromise>> mSubDialogs;

        size_t mModalLayers = 0;
    };

    struct DialogDeclined { };
    constexpr DialogDeclined dialogDeclined;

    struct MADGINE_TOOLS_EXPORT DialogContainer {
        DialogContainer() = default;
        DialogContainer(const DialogContainer &) = delete;

        DialogContainer &operator=(const DialogContainer &) = delete;

        void show(CoroutineHandle<DialogPromise> dialog);

        template <typename Dialog, typename F>
        void show(Dialog dialog, F &&f)
        {
            dialog.setCallback(std::forward<F>(f));
            show(std::move(dialog.mHandle));
        }

        void render();

    protected:
        bool renderHeader(DialogSettings &settings);
        void renderFooter(DialogSettings &settings);

        void handleDialogs(std::vector<CoroutineHandle<DialogPromise>> &dialogs);

    private:
        std::vector<CoroutineHandle<DialogPromise>> mDialogs;
    };

    template <typename... T>
    struct Dialog;
    template <typename... T>
    struct AwaitableDialog;

    struct DialogPromise {
        DialogPromise()
        {
        }

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        struct YieldSuspender {
            constexpr bool await_ready() const noexcept
            {
                return mPromise.mSettings.result.has_value() && *mPromise.mSettings.result != DialogResult::Canceled;
            }

            template <typename T>
            void await_suspend(std::coroutine_handle<T> self) const noexcept
            {
            }
            constexpr bool await_resume() const noexcept { return !mPromise.mSettings.result; }

            DialogPromise &mPromise;
        };

        std::suspend_always final_suspend() noexcept
        {
            assert(mSettings.result && *mSettings.result != DialogResult::Canceled);
            return {};
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

        DialogSettings mSettings;
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
    struct Dialog {
        struct promise_type : DialogPromise {

            Dialog<T...> get_return_object()
            {
                return { CoroutineHandle<promise_type>::fromPromise(*this) };
            }

            void return_value(std::tuple<T...> value)
            {
                if (!mSettings.result)
                    mSettings.result = DialogResult::Accepted;
                if (*mSettings.result == DialogResult::Accepted)
                    TupleUnpacker::invokeFromTuple(mCallback, value);
            }

            template <typename A>
            decltype(auto) await_transform(A &&a)
            {
                if constexpr (std::same_as<A, const get_dialog_settings_t &>) {
                    return get_dialog_settings_helper_t { *this };
                } else {
                    return std::forward<A>(a);
                }
            }

            Closure<void(T...)> mCallback;
        };

        template <typename F>
        void setCallback(F &&f)
        {
            mHandle->mCallback = std::forward<F>(f);
        }

        CoroutineHandle<promise_type> mHandle;
    };

}
}
