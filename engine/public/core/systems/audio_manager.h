#pragma once

#include "engine_sys.h"
#include "core/engine.h"
#include "core/audio/ember_audio.h"

constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE  = 48000;


/*!
 * @brief Initialize the Miniaudio audio engine.
 *
 * @version 0.0.1
 */
bool init_audio_engine();

/*!
 * @brief Close the Miniaudio audio engine.
 *
 * @version 0.0.1
 */
void close_audio_engine();


/*!
 * @brief AudioManager class
 *
 * @details Manages audio playback using Miniaudio as the backend.
 *
 * @version 0.0.1
 */
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
