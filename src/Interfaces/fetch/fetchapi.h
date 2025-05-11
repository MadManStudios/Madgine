#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/opaqueptr.h"

namespace Engine {

    struct INTERFACES_EXPORT FetchStateBase {
    FetchStateBase(std::string url, std::vector<std::string> headers);
        FetchStateBase(const FetchStateBase &) = delete;
        FetchStateBase(FetchStateBase &&other) = delete;
        ~FetchStateBase();

        void start();

        virtual void set_value() = 0;

        virtual void receive(const void *buffer, size_t count) = 0;

        FetchStateBase &operator=(const FetchStateBase &) = delete;
        FetchStateBase &operator=(FetchStateBase &&other) = delete;

        UniqueOpaquePtr mPtr1;
        UniqueOpaquePtr mPtr2;
    };

    template <typename T>
    struct FetchImpl : FetchStateBase {
        using FetchStateBase::FetchStateBase;

        void receive(const void *buffer, size_t nmemb) override;

        T mResult;
    };

    template <typename Rec, typename T>
    struct FetchState : Execution::base_state<Rec>, FetchImpl<T> {
        FetchState(Rec &&rec, std::string url, std::vector<std::string> headers)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , FetchImpl<T>(std::move(url), std::move(headers))
        {
        }

        void set_value() override
        {
            this->mRec.set_value(std::move(this->mResult));
        }
    };

    template <typename T>
    struct FetchSender : Execution::base_sender {

        template <template <typename...> typename Tuple>
        using value_types = Tuple<T>;
        using result_type = std::string;

        FetchSender(std::string url, std::vector<std::string> headers = {})
            : mUrl(std::move(url))
            , mHeaders(std::move(headers))
        {
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, FetchSender &&sender, Rec &&rec)
        {
            return FetchState<Rec, T> { std::forward<Rec>(rec), std::move(sender.mUrl), std::move(sender.mHeaders) };
        }

        std::string mUrl;
        std::vector<std::string> mHeaders;
    };

    struct JsonNull {
    };

    struct INTERFACES_EXPORT JsonObject {

        std::vector<JsonObject> &asList();
        std::map<std::string, JsonObject> &asObject();
        std::string &asString();

        std::variant<
            std::monostate,
            std::vector<JsonObject>,
            std::map<std::string, JsonObject>,
            std::string,
            int,
            bool,
            JsonNull>
            mValue;
    };

    struct INTERFACES_EXPORT JsonParser {
        JsonParser();

        operator JsonObject();

        JsonObject mRoot;

        void parse(std::string_view s);

    protected:
        bool parse(const char *&c, const char *end, JsonObject &object);

        bool parse(const char *&c, const char *end, std::vector<JsonObject> &list);
        bool parse(const char *&c, const char *end, std::map<std::string, JsonObject> &object);
        bool parse(const char *&c, const char *end, std::string &s, bool pop = true);
        bool parse(const char *&c, const char *end, int &i);
        bool parse(const char *&c, const char *end, bool &b);
        bool parse(const char *&c, const char *end, std::monostate &);
        bool parse(const char *&c, const char *end, JsonNull &);

        bool skipWs(const char *&c, const char *end);

        std::vector<JsonObject *> mStack;
        std::string mBuffer;
        bool mNeedSeparator = false;
        bool mNeedStringOpen = false;
    };

}