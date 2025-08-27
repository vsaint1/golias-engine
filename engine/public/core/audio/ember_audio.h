#pragma once

#include "core/engine.h"

constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE  = 48000;

/*!
    @brief Audio (sound resource)
*/
class Audio {
public:
    float volume   = 1.0f;
    float duration = 0.0f;

    static Audio* load(const std::string& file_Path, const std::string& busName = "Master");

    void set_volume(float vol);
    void pause();
    void play(bool loop = false);
    bool is_playing();
    void set_loop(bool loop);
    void destroy();

    Audio(const Audio&)      = delete;
    Audio& operator=(Audio&) = delete;
    Audio(Audio&&)           = delete;

private:
    ma_decoder* decoder = nullptr;
    ma_sound sound{};

    friend class AudioBus;
};

/*!
    @brief Basic AudioBus
*/
class AudioBus {
public:
    std::string _name;
    float volume = 1.0f;
    bool is_muted    = false;

    explicit AudioBus(std::string  name);

    static AudioBus& get_or_create(const std::string& name);

    void apply_volume(Audio* audio);

    void add_audio(Audio* audio);
    void remove_audio(Audio* audio);

    void set_volume(float vol);
    void set_mute(bool mute);

    std::vector<Audio*> get_sounds();

private:
    std::vector<Audio*> _sounds;
};


/*!
    @brief Engine Audio API
*/
bool init_audio_engine();
void close_audio_engine();

class AudioSystem final : public EngineSystem {
public:

    bool initialize() override;

    void update(double delta_time) override ;

    void shutdown() override;

    AudioSystem() = default;
    ~AudioSystem() override;
};
