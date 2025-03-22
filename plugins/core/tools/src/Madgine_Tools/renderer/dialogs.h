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

        std::string header = "";
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
        bool renderHeader();
        std::optional<DialogResult> renderFooter(DialogSettings settings);

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
                return false;
            }

            void await_suspend(CoroutineHandle<DialogPromise> self) const noexcept
            {
                *mPromise.mOutHandle = std::move(self);
            }
            constexpr bool await_resume() const noexcept { return !mPromise.mDone; }

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

        YieldSuspender yield_value(const DialogSettings &settings)
        {
            *mOutSettings = settings;
            return { *this };
        }

        void unhandled_exception()
        {
            throw 0;
        }

        std::optional<DialogResult> mDone;
        DialogSettings *mOutSettings = nullptr;
        CoroutineHandle<DialogPromise> *mOutHandle = nullptr;
        CoroutineHandle<DialogPromise> mResume;
        DialogContainer *mTargetContainer = nullptr;
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
                if (!mDone)
                    mDone = DialogResult::Accepted;                                
                if (*mDone == DialogResult::Accepted)
                    TupleUnpacker::invokeFromTuple(mCallback, value);
                
            }

            FinalSuspender final_suspend() noexcept
            {
                assert(mDone && *mDone != DialogResult::Canceled);                    
                return { std::move(mResume) };
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
