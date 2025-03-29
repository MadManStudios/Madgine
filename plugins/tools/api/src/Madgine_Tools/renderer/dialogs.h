#pragma once

#include "Generic/enum.h"
#include "Generic/execution/concepts.h"
#include "Generic/type_pack.h"
#include "Generic/closure.h"

namespace Engine {
namespace Tools {

    ENUM(DialogResult,
        Accepted,
        Declined,
        Canceled)

    struct DialogSettings {
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

        std::optional<DialogResult> result;
    };

    struct DialogDeclined { };
    constexpr DialogDeclined dialogDeclined;

    struct DialogPromise;

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

            void await_suspend(CoroutineHandle<DialogPromise> self) const noexcept
            {
                *mPromise.mOutHandle = std::move(self);
            }
            constexpr bool await_resume() const noexcept { return !mPromise.mSettings.result; }

            DialogPromise &mPromise;
        };

        struct FinalSuspender {
            constexpr bool await_ready() const noexcept
            {
                return !mResume;
            }

            void await_suspend(CoroutineHandle<DialogPromise> self) noexcept
            {
                self->mTargetContainer->show(std::move(mResume));                
            }
            constexpr void await_resume() const noexcept {  }

            CoroutineHandle<DialogPromise> mResume;
        };

        YieldSuspender yield_value(DialogSettings &settings)
        {
            assert(&settings == &mSettings);
            return { *this };
        }

        void unhandled_exception()
        {
            throw 0;
        }

        DialogSettings mSettings;
        CoroutineHandle<DialogPromise> *mOutHandle = nullptr;
        CoroutineHandle<DialogPromise> mResume;
        DialogContainer *mTargetContainer = nullptr;
    };

    struct get_settings_t {
        
    };
    
    constexpr get_settings_t get_settings;

    struct get_settings_helper_t {
        constexpr bool await_ready() const noexcept
        {
            return true;
        }
        void await_suspend(std::coroutine_handle<>) const noexcept {
            std::terminate();
        }
        constexpr DialogSettings& await_resume() const noexcept {
            return mPromise.mSettings;
        }

        DialogPromise &mPromise;
    };

    template <typename... T>
    struct AwaitableDialog {

        bool await_ready() const
        {
            return false;
        }

        void await_suspend(CoroutineHandle<DialogPromise> other)
        {
            mDialog.setCallback([this](T... values) { mResult = { std::forward<T>(values)... }; });
            DialogContainer *container = other->mTargetContainer;
            mDialog.mHandle->mResume = std::move(other);
            container->show(std::move(mDialog.mHandle));
        }

        auto await_resume()
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
                return { CoroutineHandle<promise_type>::fromPromise(*this) };
            }

            void return_value(std::tuple<T...> value)
            {
                if (!mSettings.result)
                    mSettings.result = DialogResult::Accepted;                                
                if (*mSettings.result == DialogResult::Accepted)
                    TupleUnpacker::invokeFromTuple(mCallback, value);
                
            }

            FinalSuspender final_suspend() noexcept
            {
                assert(mSettings.result && *mSettings.result != DialogResult::Canceled);                    
                return { std::move(mResume) };
            }

            template <typename T>
            decltype(auto) await_transform(T &&t) {
                if constexpr (std::same_as<T, const get_settings_t&>) {
                    return get_settings_helper_t { *this };
                } else {
                    return std::forward<T>(t);
                }
            }

            Closure<void(T...)> mCallback;
        };

        AwaitableDialog<T...> operator co_await() && {
            return { std::move(*this) };
        }

        template <typename F>
        void setCallback(F &&f)
        {
            mHandle->mCallback = std::forward<F>(f);
        }

        CoroutineHandle<promise_type> mHandle;
    };

}
}
