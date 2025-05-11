#include "../interfaceslib.h"

#if EMSCRIPTEN

#    include <emscripten/fetch.h>

#    include "fetchapi.h"

namespace Engine {

struct FetchData {
    emscripten_fetch_attr_t mAttributes;
    std::vector<std::string> mHeaders;
    std::vector<const char *> mHeaderPtrs;
    std::string mUrl;
};


void downloadSucceeded(emscripten_fetch_t *fetch)
{
    static_cast<FetchStateBase *>(fetch->userData)->set_value();
    emscripten_fetch_close(fetch); 
}

void downloadFailed(emscripten_fetch_t *fetch)
{
    throw 0;
    emscripten_fetch_close(fetch); // Also free data on failure.
}

void downloadProgress(emscripten_fetch_t *fetch)
{
    FetchStateBase &state = *static_cast<FetchStateBase *>(fetch->userData);

    state.receive(fetch->data, fetch->numBytes);
}

FetchStateBase::FetchStateBase(std::string url, std::vector<std::string> headers)
{
    std::unique_ptr<FetchData> ptr = std::make_unique<FetchData>();
    FetchData &data = *ptr;
    mPtr1.setupAs<std::unique_ptr<FetchData>>() = std::move(ptr);
    
    data.mUrl = std::move(url);

    data.mHeaders = std::move(headers);
    for (const std::string& s : data.mHeaders) {
        data.mHeaderPtrs.push_back(s.c_str());
    }
    data.mHeaderPtrs.push_back(nullptr);

    emscripten_fetch_attr_t &attr = data.mAttributes;

    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_STREAM_DATA;
    attr.onsuccess = downloadSucceeded;
    attr.onprogress = downloadProgress;
    attr.onerror = downloadFailed;
    attr.timeoutMSecs = 2 * 60;

    attr.userData = this;
    attr.requestHeaders = data.mHeaderPtrs.data();
}

FetchStateBase::~FetchStateBase()
{    
    mPtr1.release<std::unique_ptr<FetchData>>();
}

void FetchStateBase::start()
{
    FetchData &data = *mPtr1.as<std::unique_ptr<FetchData>>();
    emscripten_fetch(&data.mAttributes, data.mUrl.c_str());
}

}

#endif