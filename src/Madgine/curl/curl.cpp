#include "curllib.h"

#include "curl.h"

#include <curl/curl.h>

#include <cstring>

namespace Engine {

    CurlStateBase::CurlStateBase(CURLM *multiHandle, std::string url, std::vector<std::string> headers, size_t (*reader)(void *buffer, size_t size, size_t nmemb, void *self), void *self)
        : mHandle(curl_easy_init())
        , mMultiHandle(multiHandle)
    {
        curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, reader);
        curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, self);

        for (const std::string &header : headers) {
            mList = curl_slist_append(mList, header.c_str());
        }

        curl_easy_setopt(mHandle, CURLOPT_HTTPHEADER, mList);

        curl_easy_setopt(mHandle, CURLOPT_PRIVATE, this);
    }

    CurlStateBase::~CurlStateBase()
    {
        curl_slist_free_all(mList);

        if (mHandle) {
            curl_multi_remove_handle(mMultiHandle, mHandle);

            curl_easy_cleanup(mHandle);
        }
    }

    void CurlStateBase::start()
    {
        curl_multi_add_handle(mMultiHandle, mHandle);
    }

    template <>
    MADGINE_CURL_EXPORT void CurlImpl<std::string>::receive(void *buffer, size_t nmemb)
    {
        size_t oldSize = mResult.size();
        mResult.resize(oldSize + nmemb);
        strcpy_s(mResult.data() + oldSize, nmemb, static_cast<const char *>(buffer));
    }

    template <>
    MADGINE_CURL_EXPORT void CurlImpl<JsonParser>::receive(void *buffer, size_t nmemb)
    {
        mResult.parse({ static_cast<const char *>(buffer), nmemb });
    }

    template <>
    MADGINE_CURL_EXPORT void CurlImpl<std::vector<std::byte>>::receive(void *buffer, size_t nmemb)
    {
        size_t oldSize = mResult.size();
        mResult.resize(oldSize + nmemb);
        std::memcpy(mResult.data() + oldSize, buffer, nmemb);
    }

    CurlManager::CurlManager()
    {
        curl_global_init(0);

        mMultiHandle = curl_multi_init();
    }

    CurlManager::~CurlManager()
    {
        curl_multi_cleanup(mMultiHandle);

        curl_global_cleanup();
    }

    void CurlManager::update()
    {
        int runningHandles = 0;

        CURLMcode mc = curl_multi_perform(mMultiHandle, &runningHandles);
        if (mc)
            throw 0;

        int messagesInQueue = 0;

        while (CURLMsg *msg = curl_multi_info_read(mMultiHandle, &messagesInQueue)) {
            if (msg->msg != CURLMSG_DONE)
                throw 0;
            CURL *handle = msg->easy_handle;
            if (msg->data.result)
                throw 0;
            void *pointer;
            curl_easy_getinfo(handle, CURLINFO_PRIVATE, &pointer);
            static_cast<CurlStateBase *>(pointer)->set_value();
        }
    }

    JsonParser::JsonParser()
    {
        mStack.push_back(&mRoot);
    }

    JsonParser::operator JsonObject()
    {
        assert(mStack.empty());
        return std::move(mRoot);
    }

    void JsonParser::parse(std::string_view s)
    {
        if (s.empty())
            return;

        const char *c = s.data();
        const char *end = s.data() + s.size();

        if (skipWs(c, end))
            return;

        while (!parse(c, end, *mStack.back()))
            ;
    }

    bool JsonParser::parse(const char *&c, const char *end, JsonObject &object)
    {
        if (std::holds_alternative<std::monostate>(object.mValue)) {
            switch (*c) {
            case '{':
                object.mValue = std::map<std::string, JsonObject> {};
                mNeedStringOpen = true;
                ++c;
                break;
            case '[':
                object.mValue = std::vector<JsonObject> {};
                ++c;
                break;
            case '"':
                object.mValue = std::string {};
                ++c;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                object.mValue = 0;
                break;
            case 't':
            case 'f':
                object.mValue = false;
                mBuffer += *c;
                ++c;
                break;
            case 'n':
                object.mValue = JsonNull {};
                mBuffer += *c;
                ++c;
                break;
            default:
                throw 0;
            }
            if (skipWs(c, end)) {
                return true;
            }
        }
        return std::visit([&](auto &v) { return parse(c, end, v); }, object.mValue);
    }

    bool JsonParser::parse(const char *&c, const char *end, std::vector<JsonObject> &list)
    {
        if (*c == ']') {
            mStack.pop_back();
            mNeedSeparator = true;
            ++c;
            return skipWs(c, end);
        }
        if (mNeedSeparator) {
            switch (*c) {
            case ',':
                mNeedSeparator = false;
                ++c;
                if (skipWs(c, end))
                    return true;
                break;
            default:
                throw 0;
            }
        }
        mStack.push_back(&list.emplace_back());
        return skipWs(c, end);
    }

    bool JsonParser::parse(const char *&c, const char *end, std::map<std::string, JsonObject> &object)
    {
        if (*c == '}') {
            mStack.pop_back();
            mNeedSeparator = true;
            ++c;
            return skipWs(c, end);
        }
        if (mNeedSeparator) {
            switch (*c) {
            case ',':
                mNeedSeparator = false;
                mNeedStringOpen = true;
                ++c;
                if (skipWs(c, end))
                    return true;
                break;
            default:
                throw 0;
            }
        }
        if (mNeedStringOpen) {
            if (*c != '"')
                throw 0;
            mNeedStringOpen = false;
            ++c;
            if (c == end)
                return true;
        }
        if (parse(c, end, mBuffer, false))
            return true;

        switch (*c) {
        case ':':
            mStack.push_back(&object.try_emplace(std::move(mBuffer)).first->second);
            mBuffer.clear();
            ++c;
            return skipWs(c, end);
        default:
            throw 0;
        }
    }

    bool JsonParser::parse(const char *&c, const char *end, std::string &s, bool pop)
    {
        while (c != end) {
            switch (*c) {
            case '"':
                if (pop) {
                    mStack.pop_back();
                    mNeedSeparator = true;
                }
                ++c;
                return skipWs(c, end);
            default:
                s += *c;
            }
            ++c;
        }
        return true;
    }

    bool JsonParser::parse(const char *&c, const char *end, int &i)
    {
        while (c != end) {
            if (*c < '0' || *c > '9') {

                mStack.pop_back();
                mNeedSeparator = true;
                return skipWs(c, end);
            }
            i *= 10;
            i += *c - '0';
            ++c;
        }
        return true;
    }

    bool JsonParser::parse(const char *&c, const char *end, bool &b)
    {
        while (c != end) {
            if (!std::isalpha(*c)) {
                if (mBuffer == "true")
                    b = true;
                else if (mBuffer == "false")
                    b = false;
                else
                    throw 0;
                mBuffer.clear();
                mStack.pop_back();
                mNeedSeparator = true;
                return skipWs(c, end);
            }
            mBuffer += *c;
            ++c;
        }
        return true;
    }

    bool JsonParser::parse(const char *&c, const char *end, std::monostate &)
    {
        throw 0;
    }

    bool JsonParser::parse(const char *&c, const char *end, JsonNull &)
    {
        while (c != end) {
            if (!std::isalpha(*c)) {
                if (mBuffer != "null")
                    throw 0;
                mBuffer.clear();
                mStack.pop_back();
                mNeedSeparator = true;
                return skipWs(c, end);
            }
            mBuffer += *c;
            ++c;
        }
        return true;
    }

    bool JsonParser::skipWs(const char *&c, const char *end)
    {
        while (c != end && *c == ' ') {
            ++c;
        }
        return c == end;
    }

    std::vector<JsonObject> &JsonObject::asList()
    {
        return std::get<std::vector<JsonObject>>(mValue);
    }

    std::map<std::string, JsonObject> &JsonObject::asObject()
    {
        return std::get<std::map<std::string, JsonObject>>(mValue);
    }

    std::string &JsonObject::asString()
    {
        return std::get<std::string>(mValue);
    }

}