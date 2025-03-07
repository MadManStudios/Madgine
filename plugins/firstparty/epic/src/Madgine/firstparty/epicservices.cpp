#include "../epicserviceslib.h"

#include "epicservices.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include <eos_auth.h>
#include <eos_leaderboards.h>
#include <eos_logging.h>
#include <eos_sdk.h>
#include <eos_stats.h>

#include "eos_sender.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/root/root.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN_BASE(Engine::FirstParty::EpicServices, Engine::FirstParty::FirstPartyServices)
METATABLE_END(Engine::FirstParty::EpicServices)

UNIQUECOMPONENT(Engine::FirstParty::EpicServices)

namespace Engine {
namespace FirstParty {

    static std::chrono::steady_clock::duration sUpdateInterval = 15s;

    static void sLogCallback(const EOS_LogMessage *msg)
    {
        if (msg->Level == EOS_ELogLevel::EOS_LOG_Off)
            return;

        Log::MessageType type;
        switch (msg->Level) {
        case EOS_ELogLevel::EOS_LOG_Fatal:
        case EOS_ELogLevel::EOS_LOG_Error:
            type = Log::MessageType::ERROR_TYPE;
            break;
        case EOS_ELogLevel::EOS_LOG_Warning:
            type = Log::MessageType::WARNING_TYPE;
            break;
        case EOS_ELogLevel::EOS_LOG_Info:
            type = Log::MessageType::INFO_TYPE;
            break;
        case EOS_ELogLevel::EOS_LOG_Verbose:
        case EOS_ELogLevel::EOS_LOG_VeryVerbose:
            type = Log::MessageType::DEBUG_TYPE;
            break;
        default:
            throw 0;
        }
        Log::LogDummy { type } << msg->Category << ": " << msg->Message;
    }

    EpicServices::EpicServices(Root::Root &root)
        : FirstPartyServicesImpl<EpicServices>(root)
    {
        LOG("[EOS SDK] Initializing ...");

        // Init EOS SDK
        EOS_InitializeOptions SDKOptions = {};
        SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
        SDKOptions.AllocateMemoryFunction = nullptr;
        SDKOptions.ReallocateMemoryFunction = nullptr;
        SDKOptions.ReleaseMemoryFunction = nullptr;
        SDKOptions.ProductName = "Spacewar";
        SDKOptions.ProductVersion = "1.0";
        SDKOptions.Reserved = nullptr;
        SDKOptions.SystemInitializeOptions = nullptr;
        SDKOptions.OverrideThreadAffinity = nullptr;

        EOS_EResult InitResult = EOS_Initialize(&SDKOptions);
        if (InitResult != EOS_EResult::EOS_Success) {
            LOG_ERROR("[EOS SDK] Init Failed!");
            mErrorCode = -1;
            return;
        }
        LOG("[EOS SDK] Initialized.");

        EOS_EResult SetLogCallbackResult = EOS_Logging_SetCallback(&sLogCallback);
        if (SetLogCallbackResult != EOS_EResult::EOS_Success) {
            LOG_ERROR("[EOS SDK] Set Logging Callback Failed!");
        } else {
            LOG("[EOS SDK] Logging Callback Set");
            EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_Verbose);
        }

        Filesystem::Path tempPath = Filesystem::tempPath();

        // Create platform instance
        EOS_Platform_Options PlatformOptions = {};
        PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
        PlatformOptions.bIsServer = false;
        PlatformOptions.EncryptionKey = EOS_ENCYPTION_KEY;
        PlatformOptions.OverrideCountryCode = nullptr;
        PlatformOptions.OverrideLocaleCode = nullptr;
        PlatformOptions.Flags = EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D9 | EOS_PF_WINDOWS_ENABLE_OVERLAY_D3D10 | EOS_PF_WINDOWS_ENABLE_OVERLAY_OPENGL /* | EOS_PF_LOADING_IN_EDITOR*/; // Enable overlay support for D3D9/10 and OpenGL. This sample uses D3D11 or SDL.
        PlatformOptions.CacheDirectory = tempPath.c_str();

        PlatformOptions.ProductId = EOS_PRODUCT_ID;
        PlatformOptions.SandboxId = EOS_SANDBOX_ID;
        PlatformOptions.DeploymentId = EOS_DEPLOYMENT_ID;

        PlatformOptions.ClientCredentials.ClientId = EOS_CLIENT_CREDENTIALS_ID;
        PlatformOptions.ClientCredentials.ClientSecret = EOS_CLIENT_CREDENTIALS_SECRET;

        mPlatformHandle = EOS_Platform_Create(&PlatformOptions);

        if (mPlatformHandle == nullptr) {
            LOG_ERROR("[EOS SKD] Platform_Create failed!");
            mErrorCode = -1;
            return;
        }

        mAuthInterface = EOS_Platform_GetAuthInterface(mPlatformHandle);
        mConnectInterface = EOS_Platform_GetConnectInterface(mPlatformHandle);
        mLeaderboardsInterface = EOS_Platform_GetLeaderboardsInterface(mPlatformHandle);
        mStatsInterface = EOS_Platform_GetStatsInterface(mPlatformHandle);

        mInitialized = true;

        root.taskQueue()->queue([this]() -> Threading::Task<void> {
            while (mRoot.taskQueue()->running() || mRoot.taskQueue()->taskInFlightCount() > 1) {
                EOS_Platform_Tick(mPlatformHandle);
                co_await 100ms;
            }
        });

        mUserId = root.taskQueue()->queueTask(loginUser());

        mProductUserId = root.taskQueue()->queueTask(mUserId.then([this](EOS_EpicAccountId id) { return connectLogin(id); }));
    }

    EpicServices::~EpicServices()
    {
        if (mPlatformHandle)
            EOS_Platform_Release(mPlatformHandle);

        EOS_EResult result = EOS_Shutdown();
        if (result != EOS_EResult::EOS_Success) {
            LOG_ERROR("[EOS SDK] Shutdown Failed!");
        } else {
            LOG("[EOS SDK] Shutdown successful.");
        }
    }

    Threading::Task<EOS_EpicAccountId> EpicServices::loginUser()
    {
        LOG("[EOS SDK] Logging in ...");

        EOS_Auth_Credentials credentials {};

        credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;

#if EOS_LOGIN_METHOD_EXCHANGE_CODE
        throw 0;
#elif EOS_LOGIN_METHOD_EXTERNAL_AUTH
        throw 0;
#elif EOS_LOGIN_METHOD_DEV_AUTH_TOOL
        throw 0;
#elif EOS_LOGIN_METHOD_CREDENTIALS
        if (!mRetryAccountOnFailure) {
            credentials.Type = EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;
            mRetryAccountOnFailure = true;
        } else {
            mRetryAccountOnFailure = false;
            credentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
        }
#else
#    error No valid authentication method provided for EOS. Please check the variable EOS_LOGIN_METHOD in CMake.
#endif

        EOS_Auth_LoginOptions options {};
        options.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
        options.ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_BasicProfile | EOS_EAuthScopeFlags::EOS_AS_FriendsList | EOS_EAuthScopeFlags::EOS_AS_Presence;
        options.Credentials = &credentials;

        EOS_Auth_LoginCallbackInfo info = (co_await EOS_sender<EOS_Auth_Login>(mAuthInterface, &options)).value();

        switch (info.ResultCode) {
        case EOS_EResult::EOS_Success:
            LOG("[EOS SDK] Login Success!");
            co_return info.LocalUserId;
            break;
        case EOS_EResult::EOS_InvalidAuth:
        case EOS_EResult::EOS_Auth_Expired:
#if EOS_LOGIN_METHOD_CREDENTIALS
            if (mRetryAccountOnFailure) {
                co_return co_await loginUser();
                break;
            }
#endif
        default:
            LOG_ERROR("[EOS SDK] Unknown login error: " << EOS_EResult_ToString(info.ResultCode));
            co_return nullptr;
        }
    }

    Threading::Task<EOS_ProductUserId> EpicServices::connectLogin(EOS_EpicAccountId id)
    {
        LOG("[EOS SDK] Connect Login - User ID: "
            << "<TODO>");

        EOS_Auth_Token *userAuthToken = nullptr;

        assert(mAuthInterface != nullptr);

        EOS_Auth_CopyUserAuthTokenOptions copyTokenOptions = { 0 };
        copyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

        if (EOS_Auth_CopyUserAuthToken(mAuthInterface, &copyTokenOptions, id, &userAuthToken) == EOS_EResult::EOS_Success) {
            EOS_Connect_Credentials Credentials;
            Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
            Credentials.Token = userAuthToken->AccessToken;
            Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;

            EOS_Connect_LoginOptions Options = { 0 };
            Options.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
            Options.Credentials = &Credentials;
            Options.UserLoginInfo = nullptr;

            assert(mConnectInterface != nullptr);
            EOS_Connect_LoginCallbackInfo info = (co_await EOS_sender<EOS_Connect_Login>(mConnectInterface, &Options)).value();
            EOS_Auth_Token_Release(userAuthToken);

            switch (info.ResultCode) {
            case EOS_EResult::EOS_Success:
                LOG("[EOS SDK] Connect Login Success!");
                {
                    char buffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
                    int32_t bufferSize = sizeof(buffer);
                    EOS_EResult Result = EOS_ProductUserId_ToString(info.LocalUserId, buffer, &bufferSize);

                    if (Result == EOS_EResult::EOS_Success) {
                        LOG("Product User ID: " << buffer);
                    }
                }
                co_return info.LocalUserId;
                break;
            default:
                LOG_ERROR("[EOS SDK] Unknown connect login error: " << EOS_EResult_ToString(info.ResultCode));
                co_return nullptr;
            }
        }

        co_return nullptr;
    }

    std::string_view EpicServices::key() const
    {
        return "EpicServices";
    }

    std::string EpicServices::currentUserName() const
    {
        throw 0;
    }

    Threading::Task<Leaderboard> EpicServices::getLeaderboardTask(const char *name, Leaderboard::AccessMode accessmode, Leaderboard::ReferenceRank referenceRank, int32_t rangeBegin, int32_t rangeEnd)
    {
        EOS_ProductUserId userId = co_await mProductUserId;
        if (!userId) {
            co_return {};
        }

        auto pib = mCache.try_emplace({ name, accessmode });
        Leaderboard &leaderboard = pib.first->second.mBoard;

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (pib.second || now - pib.first->second.mLastUpdate > sUpdateInterval) {
            EOS_Leaderboards_QueryLeaderboardRanksOptions options {};
            options.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
            options.LeaderboardId = name;
            options.LocalUserId = userId;

            EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo info = (co_await EOS_sender<EOS_Leaderboards_QueryLeaderboardRanks>(mLeaderboardsInterface, &options)).value();

            if (info.ResultCode != EOS_EResult::EOS_Success) {
                LOG_ERROR("[EOS SDK] Leaderboards - Query Leaderboard Ranks Error: " << EOS_EResult_ToString(info.ResultCode));
                co_return {};
            }

            EOS_Leaderboards_GetLeaderboardRecordCountOptions leaderboardsRecordsCountOptions {};
            leaderboardsRecordsCountOptions.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST;

            uint32_t leaderboardRecordsCount = EOS_Leaderboards_GetLeaderboardRecordCount(mLeaderboardsInterface, &leaderboardsRecordsCountOptions);

            leaderboard.mEntries.clear();
            pib.first->second.mLastUpdate = now;

            EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions copyOptions {};
            copyOptions.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST;

            for (copyOptions.LeaderboardRecordIndex = 0; copyOptions.LeaderboardRecordIndex < leaderboardRecordsCount; ++copyOptions.LeaderboardRecordIndex) {
                EOS_Leaderboards_LeaderboardRecord *leaderboardRecord = NULL;
                EOS_EResult copyLeaderboardRecordResult = EOS_Leaderboards_CopyLeaderboardRecordByIndex(mLeaderboardsInterface, &copyOptions, &leaderboardRecord);
                if (copyLeaderboardRecordResult != EOS_EResult::EOS_Success) {
                    LOG_ERROR("CopyLeaderboardRecords Failure!");
                    break;
                }

                Leaderboard::Entry &entry = leaderboard.mEntries.emplace_back();

                entry.mUserId = reinterpret_cast<uintptr_t>(leaderboardRecord->UserId);
                entry.mDisplayName = leaderboardRecord->UserDisplayName;
                entry.mRank = leaderboardRecord->Rank;
                entry.mScore = leaderboardRecord->Score;

                // Release Leaderboard Record
                EOS_Leaderboards_LeaderboardRecord_Release(leaderboardRecord);
            }
        }

        std::vector<Leaderboard::Entry>::iterator it;

        switch (referenceRank) {
        case Leaderboard::FIRST:
            it = leaderboard.mEntries.begin();
            break;
        case Leaderboard::USER:
            it = std::ranges::find(leaderboard.mEntries, reinterpret_cast<uintptr_t>(userId), &Leaderboard::Entry::mUserId);
            if (it == leaderboard.mEntries.end())
                it = leaderboard.mEntries.begin();
            break;
        default:
            co_return {};
        }

        rangeBegin = std::max<int32_t>(rangeBegin, -std::distance(leaderboard.mEntries.begin(), it));
        rangeEnd = std::min<int32_t>(rangeEnd, std::distance(it, leaderboard.mEntries.end()));

        Leaderboard result;

        result.mEntries = { it + rangeBegin, it + rangeEnd };

        co_return result;
    }

    Threading::Task<std::vector<EpicServices::LeaderboardInfo>> EpicServices::getLeaderboards()
    {
        EOS_ProductUserId userId = co_await mProductUserId;
        if (!userId) {
            co_return {};
        }

        LOG("[EOS SDK] Leaderboards - Query Definitions ...");

        EOS_Leaderboards_QueryLeaderboardDefinitionsOptions QueryDefinitionsOptions = { 0 };
        QueryDefinitionsOptions.ApiVersion = EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST;
        QueryDefinitionsOptions.StartTime = EOS_LEADERBOARDS_TIME_UNDEFINED;
        QueryDefinitionsOptions.EndTime = EOS_LEADERBOARDS_TIME_UNDEFINED;
        QueryDefinitionsOptions.LocalUserId = userId;

        EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo info = (co_await EOS_sender<EOS_Leaderboards_QueryLeaderboardDefinitions>(mLeaderboardsInterface, &QueryDefinitionsOptions)).value();

        if (info.ResultCode != EOS_EResult::EOS_Success) {
            LOG_ERROR("[EOS SDK] Leaderboards - Query Definitions Error: " << EOS_EResult_ToString(info.ResultCode));
            co_return {};
        }

        LOG("[EOS SDK] Leaderboards - Query Definitions Complete");

        EOS_Leaderboards_GetLeaderboardDefinitionCountOptions LeaderboardDefinitionsCountOptions = { 0 };
        LeaderboardDefinitionsCountOptions.ApiVersion = EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST;

        uint32_t LeaderboardDefinitionsCount = EOS_Leaderboards_GetLeaderboardDefinitionCount(mLeaderboardsInterface, &LeaderboardDefinitionsCountOptions);

        EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions CopyOptions = { 0 };
        CopyOptions.ApiVersion = EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST;

        // Clear definitions
        std::vector<LeaderboardInfo> leaderboards;

        for (CopyOptions.LeaderboardIndex = 0; CopyOptions.LeaderboardIndex < LeaderboardDefinitionsCount; ++CopyOptions.LeaderboardIndex) {
            EOS_Leaderboards_Definition *LeaderboardDef = nullptr;
            EOS_EResult CopyLeaderboardDefinitionsResult = EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(mLeaderboardsInterface, &CopyOptions, &LeaderboardDef);
            if (CopyLeaderboardDefinitionsResult != EOS_EResult::EOS_Success) {
                LOG_ERROR("[EOS SDK] Leaderboards - CopyLeaderboardDefinitions Failure!");
                break;
            }

            LeaderboardInfo &leaderboard = leaderboards.emplace_back();

            if (LeaderboardDef->LeaderboardId) {
                leaderboard.mId = LeaderboardDef->LeaderboardId;
            }

            if (LeaderboardDef->StatName) {
                leaderboard.mStatName = LeaderboardDef->StatName;
            }

            /* LeaderboardsDefinition->StartTime = LeaderboardDef->StartTime;

            LeaderboardsDefinition->EndTime = LeaderboardDef->EndTime;*/

            switch (LeaderboardDef->Aggregation) {
            case EOS_ELeaderboardAggregation::EOS_LA_Min:
                leaderboard.mAggregation = LeaderboardInfo::MIN;
                break;
            case EOS_ELeaderboardAggregation::EOS_LA_Max:
                leaderboard.mAggregation = LeaderboardInfo::MAX;
                break;
            case EOS_ELeaderboardAggregation::EOS_LA_Sum:
                leaderboard.mAggregation = LeaderboardInfo::SUM;
                break;
            case EOS_ELeaderboardAggregation::EOS_LA_Latest:
                leaderboard.mAggregation = LeaderboardInfo::LATEST;
                break;
            }

            // Release Leaderboard Definition
            EOS_Leaderboards_Definition_Release(LeaderboardDef);
        }

        co_return leaderboards;
    }

    Threading::Task<bool> EpicServices::ingestStatTask(const char *name, const char *leaderboardName, int32_t value)
    {
        EOS_ProductUserId userId = co_await mProductUserId;
        if (!userId) {
            co_return {};
        }

        EOS_Stats_IngestStatOptions options {};
        options.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
        options.LocalUserId = userId;
        options.TargetUserId = userId;
        options.StatsCount = 1;

        EOS_Stats_IngestData data {};
        data.ApiVersion = EOS_STATS_INGESTDATA_API_LATEST;
        data.StatName = name;
        data.IngestAmount = value;

        options.Stats = &data;

        EOS_Stats_IngestStatCompleteCallbackInfo info = (co_await EOS_sender<EOS_Stats_IngestStat>(mStatsInterface, &options)).value();

        if (info.ResultCode != EOS_EResult::EOS_Success) {
            LOG_ERROR("[EOS SDK] Stats - Ingest Stats Error: " << EOS_EResult_ToString(info.ResultCode));
            co_return false;
        }

        LOG("[EOS SDK] Stats - Stats Ingest Complete");
        co_return true;
    }

    Threading::Task<bool> EpicServices::unlockAchievementTask(const char *name)
    {
        throw 0;
    }

    Threading::Task<std::vector<Lobby>> EpicServices::getLobbyListTask()
    {
        throw 0;
    }

    Threading::Task<std::optional<Lobby>> EpicServices::createLobbyTask(size_t maxPlayerCount, MatchmakingCallback cb, SessionStartedCallback sessionCb, std::map<std::string, std::string> properties)
    {
        throw 0;
    }

    Threading::Task<std::optional<Lobby>> EpicServices::joinLobbyTask(uint64_t id, MatchmakingCallback cb, SessionStartedCallback sessionCb)
    {
        throw 0;
    }

    Threading::Task<std::optional<ServerInfo>> EpicServices::startMatchTask()
    {
        throw 0;
    }

    void EpicServices::leaveLobby()
    {
        throw 0;
    }

    void EpicServices::leaveMatch()
    {
        throw 0;
    }

    bool EpicServices::isLobbyOwner() const
    {
        throw 0;
    }

    void Engine::FirstParty::EpicServices::setLobbyProperty(std::string_view key, std::string_view value)
    {
        throw 0;
    }

}
}
