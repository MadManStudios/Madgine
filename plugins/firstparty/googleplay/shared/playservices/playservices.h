

#if defined(Im3D_EXPORTS)
#    define MADGINE_PLAY_SERVICES_EXPORT DLL_EXPORT
#else
#    define MADGINE_PLAY_SERVICES_EXPORT DLL_IMPORT
#endif

#include "Generic/execution/virtualsender.h"

namespace PlayServices {

MADGINE_PLAY_SERVICES_EXPORT void setup();

namespace Achievements {
    MADGINE_PLAY_SERVICES_EXPORT void unlockImpl(Engine::Execution::VirtualReceiverBase<std::string> &rec, std::string_view id);
    inline auto unlock(std::string_view id)
    {
        return Engine::Execution::make_simple_virtual_sender<std::string>(&unlockImpl, std::move(id));
    }
}
namespace Leaderboards {
    struct Scores {
        struct Score {
            uintptr_t mUserId;
            std::string mDisplayName;
            size_t mRank;
            int32_t mScore;
        };
        std::vector<Score> mScores;
    };
    struct ScoresState : Engine::Execution::VirtualReceiverBase<std::string, Scores> {

        ScoresState(std::string_view id, int accessmode, int referenceRank, int rangeBegin, int rangeEnd)
            : mId(id)
            , mAccessmode(accessmode)
            , mReferenceRank(referenceRank)
            , mRangeBegin(rangeBegin)
            , mRangeEnd(rangeEnd)
        {
        }

        void start();

        Scores mScores;

        std::string_view mId;
        int mAccessmode;
        int mReferenceRank;
        int mRangeBegin;
        int mRangeEnd;
    };    
    inline auto getLeaderboard(std::string_view id, int accessmode, int referenceRank, int rangeBegin, int rangeEnd)
    {
        return Engine::Execution::make_virtual_sender<ScoresState>(std::move(id), std::move(accessmode), std::move(referenceRank), std::move(rangeBegin), std::move(rangeEnd));
    }
    MADGINE_PLAY_SERVICES_EXPORT void submitScoreImpl(Engine::Execution::VirtualReceiverBase<std::string> &rec, std::string_view leaderboardId, int32_t score, std::string_view name);
    inline auto submitScore(std::string_view leaderboardId, int32_t score, std::string_view name)
    {
        return Engine::Execution::make_simple_virtual_sender<std::string>(&submitScoreImpl, std::move(leaderboardId), std::move(score), std::move(name));
    }
}

}
