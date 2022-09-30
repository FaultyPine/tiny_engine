#include "tiny_audio.h"

#include "pch.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static ma_engine globalAudioEngine;

void InitAudioEngine() {
    ma_result result;
    result = ma_engine_init(NULL, &globalAudioEngine);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize audio engine.\n";
    }
}
void PlayAudio(const char* file)  {
    ma_engine_play_sound(&globalAudioEngine, file, NULL);
}