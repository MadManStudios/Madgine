#include "../interfaceslib.h"

#if !EMSCRIPTEN

#    include <curl/curl.h>

#    include <cstring>

#    include "fetchapi.h"

namespace Engine {

struct CurlManager {
    CurlManager()
    {
        curl_global_init(0);

        mHandle = curl_multi_init();

        mThread = std::thread { &CurlManager::update, this };
    }

    ~CurlManager()
    {
        mStopSource.request_stop();

        curl_multi_wakeup(mHandle);

        mThread.join();

        curl_multi_cleanup(mHandle);

        curl_global_cleanup();
    }

    void update()
    {
        while (!mStopSource.stop_requested()) {
            int runningHandles = 0;

            CURLMcode mc = curl_multi_perform(mHandle, &runningHandles);
            if (mc)
                throw 0;

            int messagesInQueue = 0;

            while (CURLMsg *msg = curl_multi_info_read(mHandle, &messagesInQueue)) {
                if (msg->msg != CURLMSG_DONE)
                    throw 0;
                CURL *handle = msg->easy_handle;
                if (msg->data.result)
                    throw 0;
                void *pointer;
                curl_easy_getinfo(handle, CURLINFO_PRIVATE, &pointer);
                static_cast<FetchStateBase *>(pointer)->set_value();
            }

            mc = curl_multi_poll(mHandle, nullptr, 0, 200000000, nullptr);
            if (mc)
                throw 0;
        }
    }

    CURLM *mHandle = nullptr;
    std::thread mThread;
    std::stop_source mStopSource;
};

static CURLM *sHandle()
{
    static CurlManager mgr;

    return mgr.mHandle;
}

FetchStateBase::FetchStateBase(std::string url, std::vector<std::string> headers)
{
    CURL *handle = curl_easy_init();

    mPtr1.setupAs<CURL *>() = handle;

    auto reader = [](void *buffer, size_t size, size_t nmemb, void *self) -> size_t {
        assert(size == 1);
        static_cast<FetchStateBase *>(self)->receive(buffer, nmemb);
        return nmemb;
    };

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, static_cast<size_t (*)(void *, size_t, size_t, void *)>(reader));
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

    curl_slist *list = nullptr;

    for (const std::string &header : headers) {
        list = curl_slist_append(list, header.c_str());
    }

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);

    mPtr2.setupAs<curl_slist *>() = list;

    curl_easy_setopt(handle, CURLOPT_PRIVATE, this);
}

FetchStateBase::~FetchStateBase()
{
    curl_slist_free_all(mPtr2.release<curl_slist *>());

    if (mPtr1) {
        CURL *handle = mPtr1.release<CURL *>();

        curl_multi_remove_handle(sHandle(), handle);

        curl_easy_cleanup(handle);
    }
}

void FetchStateBase::start()
{
    curl_multi_add_handle(sHandle(), mPtr1.as<CURL *>());
    curl_multi_wakeup(sHandle());
}

}

#endif