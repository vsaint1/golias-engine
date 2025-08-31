#pragma once

#include "engine_sys.h"
#include "core/engine.h"
#include "core/audio/ember_audio.h"

constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE  = 48000;


/*!
    @brief Engine Audio API
*/
bool init_audio_engine();

void close_audio_engine();

class AudioManager final : public EngineManager {
public:

    bool initialize() override;

    void update(double delta_time) override ;

    void shutdown() override;

    AudioManager() = default;
    ~AudioManager() override;
protected:
    const char* name = "AudioSystem";
};
