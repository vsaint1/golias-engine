#include "audio/audio.h"

#include "core/engine.h"
#include <miniaudio.h>

namespace golias {
    Audio::~Audio() {
        // if (maSound) {
        //     ma_sound_uninit(maSound.get());
        // }
        
        // if (maDecoder) {
        //     ma_decoder_uninit(maDecoder.get());
        // }
    }

    void Audio::SetPosition(float x, float y, float z) {
        if (maSound) {
            ma_sound_set_position(maSound.get(), x, y, z);
        }
    }

    void Audio::Play(bool loop) {
        if (!maSound) {
            return;
        }

        if (loop) {

            ma_sound_set_looping(maSound.get(), MA_TRUE);
            ma_sound_set_volume(maSound.get(), volume);
            if (!ma_sound_is_playing(maSound.get())) {
                ma_sound_start(maSound.get());
            }

        } else {

            PlayFireAndForget();
        }
    }

    void Audio::OnSoundEnd(void* pUserData, ma_sound* pSound) {
        if (pSound) {
            ma_sound_uninit(pSound);
            delete pSound;
        }
    }

    void Audio::PlayFireAndForget() {

        auto& audioManager = Engine::GetInstance().GetAudioManager();
        auto fullPath      = Engine::GetInstance().GetFileSystem().GetAssetsPath() + path;

        ma_engine_set_volume(audioManager.GetNativeHandle(), volume);
        ma_engine_play_sound(audioManager.GetNativeHandle(), fullPath.c_str(), 0);

        // if (buffer.empty()) {
        //     return;
        // }

        // auto& audioManager = Engine::GetInstance().GetAudioManager();

        // ma_decoder* fnfDecoder = new ma_decoder();
        // auto res               = ma_decoder_init_memory(buffer.data(), buffer.size(), NULL, fnfDecoder);

        // if (res != MA_SUCCESS) {
        //     delete fnfDecoder;
        //     return;
        // }

        // ma_sound* fnfSound = new ma_sound();
        // res = ma_sound_init_from_data_source(audioManager.GetNativeHandle(), &fnfDecoder->ds, MA_SOUND_FLAG_DECODE, NULL, fnfSound);

        // if (res != MA_SUCCESS) {
        //     ma_decoder_uninit(fnfDecoder);
        //     delete fnfDecoder;
        //     delete fnfSound;
        //     return;
        // }

        // ma_sound_set_volume(fnfSound, volume);
        // ma_sound_set_spatialization_enabled(fnfSound, spatializationEnabled ? MA_TRUE : MA_FALSE);

        // ma_sound_set_end_callback(fnfSound, OnSoundEnd, fnfDecoder);

        // ma_sound_start(fnfSound);
    }

    void Audio::SetSpatialization(bool enabled) {
        spatializationEnabled = enabled;
        if (maSound) {
            ma_sound_set_spatialization_enabled(maSound.get(), enabled ? MA_TRUE : MA_FALSE);
        }
    }

    void Audio::Stop() {
        if (maSound && ma_sound_is_playing(maSound.get())) {
            ma_sound_stop(maSound.get());
        }
    }

    void Audio::SetVolume(float value) {
        volume = value;
        if (maSound) {
            ma_sound_set_volume(maSound.get(), volume);
        }
    }

    float Audio::GetVolume() const {
        return volume;
    }

    bool Audio::IsPlaying() const {
        return maSound && ma_sound_is_playing(maSound.get());
    }

    std::shared_ptr<Audio> Audio::Load(const std::string_view pPath) {

        auto data = Engine::GetInstance().GetFileSystem().LoadAssetFile(pPath);
        if (data.empty()) {
            spdlog::error("Audio::Load Failed to load audio file: {}", pPath);
            return nullptr;
        }

        auto& audioManager = Engine::GetInstance().GetAudioManager();
        auto audio         = std::make_shared<Audio>();
        audio->buffer      = std::move(data);
        audio->path        = pPath.data();
        audio->maDecoder   = std::make_unique<ma_decoder>();

        auto res = ma_decoder_init_memory(audio->buffer.data(), audio->buffer.size(), NULL, audio->maDecoder.get());

        if (res != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to initialize decoder for: {}", pPath);
            return nullptr;
        }

        audio->maSound = std::make_unique<ma_sound>();
        res            = ma_sound_init_from_data_source(
            audioManager.GetNativeHandle(), &audio->maDecoder->ds, MA_SOUND_FLAG_DECODE, NULL, audio->maSound.get());

        if (res != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to initialize sound for: {}", pPath);
            return nullptr;
        }

        spdlog::info("Audio::Load Successfully loaded audio file: {}", pPath);
        return audio;
    }
} // namespace golias
