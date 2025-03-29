#pragma once

namespace Engine {
namespace Audio {

	MADGINE_AUDIO_EXPORT Stream DecodeOggFile(AudioInfo &info, Stream &&file);

}
}