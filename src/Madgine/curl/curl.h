#pragma once

#include "Generic/execution/concepts.h"

using CURL = void;
using CURLM = void;
struct curl_slist;

namespace Engine {

    struct MADGINE_CURL_EXPORT CurlStateBase {
        CurlStateBase(CURLM *multiHandle, std::string url, std::vector<std::string> headers, size_t (*reader)(void *buffer, size_t size, size_t nmemb, void *self), void *self);
        CurlStateBase(const CurlStateBase &) = delete;
        CurlStateBase(CurlStateBase &&other) = delete;
        ~CurlStateBase();

        void start();

        virtual void set_value() = 0;

        CurlStateBase &operator=(const CurlStateBase &) = delete;
        CurlStateBase &operator=(CurlStateBase &&other) = delete;

        CURLM *mMultiHandle;
        CURL *mHandle;
        curl_slist *mList = nullptr;
    };

    template <typename T>
    struct CurlImpl : CurlStateBase {
        CurlImpl(CURLM *multiHandle, std::string url, std::vector<std::string> headers)
            : CurlStateBase(multiHandle, std::move(url), std::move(headers), helper, this)
        {
        }

        void receive(void *buffer, size_t nmemb);

        static size_t helper(void *buffer, size_t size, size_t nmemb, void *self)
        {
            assert(size == 1);
            static_cast<CurlImpl<T> *>(self)->receive(buffer, nmemb);
            return nmemb;
        }

        T mResult;
    };

    template <typename Rec, typename T>
    struct CurlState : Execution::base_state<Rec>, CurlImpl<T> {
        CurlState(Rec &&rec, CURLM *multiHandle, std::string url, std::vector<std::string> headers)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , CurlImpl<T>(multiHandle, std::move(url), std::move(headers))
        {
        }

        void set_value() override
        {
            this->mRec.set_value(std::move(this->mResult));
        }
    };

    template <typename T>
    struct CurlSender : Execution::base_sender {

        template <template <typename...> typename Tuple>
        using value_types = Tuple<T>;
        using result_type = std::string;

        CurlSender(CURLM *multiHandle, std::string url, std::vector<std::string> headers = {})
            : mMultiHandle(multiHandle)
            , mUrl(std::move(url))
            , mHeaders(std::move(headers))
        {
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, CurlSender &&sender, Rec &&rec)
        {
            return CurlState<Rec, T> { std::forward<Rec>(rec), sender.mMultiHandle, std::move(sender.mUrl), std::move(sender.mHeaders) };
        }

        CURLM *mMultiHandle;
        std::string mUrl;
        std::vector<std::string> mHeaders;
    };

    struct MADGINE_CURL_EXPORT CurlManager {
        CurlManager();
        ~CurlManager();

        void update();

        template <typename T>
        auto request(std::string url, std::vector<std::string> headers)
        {
            return CurlSender<T> { mMultiHandle, std::move(url), std::move(headers) };
        }

        CURLM *mMultiHandle = nullptr;
    };

    struct JsonNull {
    };

    struct MADGINE_CURL_EXPORT JsonObject {

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

    struct MADGINE_CURL_EXPORT JsonParser {
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