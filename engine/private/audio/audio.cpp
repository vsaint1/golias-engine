#include "audio/audio.h"

#include "core/engine.h"
#include <miniaudio.h>

namespace golias {
    Audio::~Audio() {
       
        if (maDecoder) {
            ma_decoder_uninit(maDecoder.get());
        }

         if (maSound) {
            ma_sound_uninit(maSound.get());
        }
    }

    void Audio::SetPosition(float x, float y, float z) {
        if (maSound) {
            ma_sound_set_position(maSound.get(), x, y, z);
        }
    }

    void Audio::Play(bool loop) {
        if (maSound) {
            ma_sound_set_looping(maSound.get(), loop ? MA_TRUE : MA_FALSE);
            ma_sound_set_volume(maSound.get(), volume);
     
            if (ma_sound_at_end(maSound.get()) || !ma_sound_is_playing(maSound.get())) {
                ma_sound_start(maSound.get());
            } else {
                ma_sound_stop(maSound.get());
                ma_sound_start(maSound.get());
            }
        }
    }

    void Audio::SetSpatialization(bool enabled) {
        if (maSound) {
            ma_sound_set_spatialization_enabled(maSound.get(), enabled ? MA_TRUE : MA_FALSE);
        }
    }

    void Audio::Stop() {
        if (maSound) {
            if (IsPlaying()) {
                ma_sound_stop(maSound.get());
                // ma_sound_seek_to_pcm_frame(maSound.get(), 0);
            }
        }
    }


    void Audio::SetVolume(float value) {
        volume = value;
        if (maSound) {
            ma_sound_set_volume(maSound.get(), volume);
        }
    }

    float Audio::GetVolume() const {
        if (maSound) {
            return ma_sound_get_volume(maSound.get());
        }

        return 0.0f;
    }

    bool Audio::IsPlaying() const {
        if (maSound) {
            return ma_sound_is_playing(maSound.get());
        }

        return false;
    }

    std::shared_ptr<Audio> Audio::Load(const std::string_view pPath) {
        auto data = Engine::GetInstance().GetFileSystem().LoadAssetFile(pPath);

        if (data.empty()) {
            spdlog::error("Audio::Load Failed to load audio file: {}", pPath);
            return nullptr;
        }

        auto& audioManager = Engine::GetInstance().GetAudioManager();


        auto audio       = std::make_shared<Audio>();
        audio->maSound   = std::make_unique<ma_sound>();
        audio->buffer    = std::move(data);
        audio->maDecoder = std::make_unique<ma_decoder>();

        auto res = ma_decoder_init_memory(audio->buffer.data(), audio->buffer.size(), NULL, audio->maDecoder.get());

        if (res != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to initialize decoder for audio file: {}", pPath);
            return nullptr;
        }

        res = ma_sound_init_from_data_source(audioManager.GetNativeHandle(), &audio->maDecoder->ds, NULL, NULL, audio->maSound.get());

        if (res != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to decode  for audio file: {}", pPath);

            return nullptr;
        }
        
     
        // ma_sound_set_spatialization_enabled(audio->maSound.get(), MA_TRUE);

        spdlog::info("Audio::Load Successfully loaded audio file: {}", pPath);
        return audio;
    }
} // namespace golias
