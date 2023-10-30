#ifndef TINY_AUDIO_H
#define TINY_AUDIO_H

//#include "pch.h"
#include "tiny_defines.h"

namespace Audio {

TAPI void InitAudioEngine();
TAPI void PlayAudio(const char* file);
TAPI void SetMute(bool mute);
TAPI void SetVolume(f32 volume);

} 

#endif