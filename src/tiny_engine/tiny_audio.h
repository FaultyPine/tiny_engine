#ifndef TINY_AUDIO_H
#define TINY_AUDIO_H

namespace Audio {

void InitAudioEngine();
void PlayAudio(const char* file);
void SetMute(bool mute);

}

#endif