#pragma once

/// @cond

#include "Madgine/audiolib.h"

#if defined(PortAudioApi_EXPORTS)
#    define MADGINE_PORTAUDIO_API_EXPORT DLL_EXPORT
#else
#    define MADGINE_PORTAUDIO_API_EXPORT DLL_IMPORT
#endif

#include "portaudioapiforward.h"



/// @endcond
