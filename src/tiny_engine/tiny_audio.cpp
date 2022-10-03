#include "tiny_audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace Audio {

static ma_engine globalAudioEngine;
static bool isMuted = false;

void InitAudioEngine() {
    ma_result result;
    result = ma_engine_init(NULL, &globalAudioEngine);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize audio engine.\n";
    }
}
void PlayAudio(const char* file)  {
    if (!isMuted) {
        ma_engine_play_sound(&globalAudioEngine, file, NULL);
    }
}

void SetMute(bool mute) {
    isMuted = mute;
}

void SetVolume(f32 volume) {
    ma_engine_set_volume(&globalAudioEngine, volume);
}


} // namespace Audio