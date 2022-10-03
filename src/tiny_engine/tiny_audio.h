#ifndef TINY_AUDIO_H
#define TINY_AUDIO_H

#include "pch.h"

namespace Audio {

void InitAudioEngine();
void PlayAudio(const char* file);
void SetMute(bool mute);
void SetVolume(f32 volume);

}

#endif