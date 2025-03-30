#include "Generic/genericlib.h"

#if defined(MadgineCurl_EXPORTS)
#    define MADGINE_CURL_EXPORT DLL_EXPORT
#else
#    define MADGINE_CURL_EXPORT DLL_IMPORT
#endif

#include "curlforward.h"