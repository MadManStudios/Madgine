#pragma once

#include "Generic/execution/sender.h"
#include "Generic/genericresult.h"

namespace Engine {
namespace FirstParty {

    template <typename R, typename Rec>
    struct steam_state {
        void start()
        {
            mCallResult.Set(mCall, this, &steam_state::callback);
        }
        void callback(R *result, bool bIOFailure)
        {
            mRec.set_value(*result);
        }
        Rec mRec;
        SteamAPICall_t mCall;
        CCallResult<steam_state, R> mCallResult;           
    };

    template <typename R>
    auto steam_sender(SteamAPICall_t call)
    {
        return Execution::make_sender<GenericResult, R>(
            [=]<typename Rec>(Rec &&rec) {
                return steam_state<R, Rec> { std::forward<Rec>(rec), call };
            });
    }

}
}