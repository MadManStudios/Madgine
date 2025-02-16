#include "../googleplayserviceslib.h"

#include "googleplayservices.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

//#include <steam/steam_api.h>

//#include "steam_sender.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/root/root.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "playservices/playservices.h"

#include "Generic/execution/execution.h"

METATABLE_BEGIN_BASE(Engine::FirstParty::GooglePlayServices, Engine::FirstParty::FirstPartyServices)
METATABLE_END(Engine::FirstParty::GooglePlayServices)

UNIQUECOMPONENT(Engine::FirstParty::GooglePlayServices)

namespace Engine {
namespace FirstParty {

    GooglePlayServices::GooglePlayServices(Root::Root &root)
        : FirstPartyServicesImpl<GooglePlayServices>(root)
    {
        mInitialized = PlayServices::setup();        
    }

    GooglePlayServices::~GooglePlayServices()
    {
    }

    std::string_view GooglePlayServices::key() const
    {
        return "GooglePlayServices";
    }

    Threading::Task<Leaderboard> GooglePlayServices::getLeaderboardTask(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd)
    {
        WithResult<std::string, PlayServices::Leaderboards::Scores> result = co_await PlayServices::Leaderboards::getLeaderboard(name, accessmode, referenceRank, rangeBegin, rangeEnd);
        PlayServices::Leaderboards::Scores scores;
        std::string errorMsg = result.get(scores);
        if (!errorMsg.empty()) {
            LOG_ERROR("Error fetching Leaderboard " << name << ": \n "
                                             << errorMsg);            
        }
        Leaderboard transferredScores;

        std::ranges::transform(scores.mScores, std::back_inserter(transferredScores.mEntries), [](const PlayServices::Leaderboards::Scores::Score &score) {
            return Leaderboard::Entry {
                score.mUserId,
                score.mDisplayName,
                score.mRank,
                score.mScore
            };
        });

        co_return transferredScores;        
    }

    Threading::Task<bool> GooglePlayServices::ingestStatTask(const char *name, const char *leaderboardName, int32_t value)
    {
        std::optional<std::string> result = co_await PlayServices::Leaderboards::submitScore(leaderboardName, value, name); 
        if (result) {
            LOG_ERROR("Error updating Stat " << name << " in Leaderboard " << leaderboardName << ": \n "
                << *result);
            co_return false;
        } 
        co_return false;
    }

    Threading::Task<bool> GooglePlayServices::unlockAchievementTask(const char *name)
    {
        std::optional<std::string> result = co_await PlayServices::Achievements::unlock(name);
        if (result) {
            LOG_ERROR("Error unlocking Achievement: \n" << *result);
            co_return false;
        } 
        co_return true;
    }

    Threading::Task<std::vector<Lobby>> GooglePlayServices::getLobbyListTask()
    {
        throw 0;
    }

}
}
