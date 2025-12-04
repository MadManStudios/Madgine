#include "../interfaceslib.h"

#if ANDROID

#    include "fetchapi.h"

namespace Engine {

FetchStateBase::FetchStateBase(std::string url, std::vector<std::string> headers)
{
}

FetchStateBase::~FetchStateBase()
{
}

void FetchStateBase::start()
{
    set_error(GenericResult::UNKNOWN_ERROR);
}

void FetchStateBase::stop()
{
    throw 0;
}

}

#endif