#include "core/audio/audio.h"

#include "core/engine.h"
#include <miniaudio.h>


namespace golias {

    Audio::~Audio() {
        if (mSound) {
            ma_sound_uninit(mSound.get());
            mSound.reset();
        }

        if (mDecoder) {
            ma_decoder_uninit(mDecoder.get());
            mDecoder.reset();
        }
    }

    Ref<Audio> Audio::Load(CString path) {

        auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);

        ma_engine* engine = Engine::GetInstance().GetAudioManager().GetEngine();

        Ref<Audio> audio  = std::make_shared<Audio>();
        audio->mAudioData = std::move(buffer);
        audio->mDecoder   = std::make_unique<ma_decoder>();
        audio->mSound     = std::make_unique<ma_sound>();

        ma_result result = ma_decoder_init_memory(audio->mAudioData.data(), audio->mAudioData.size(), NULL, audio->mDecoder.get());

        if (result != MA_SUCCESS) {
            GOLIAS_LOG_ERROR("Failed to initialize audio decoder for file: %s", path.data());
            return nullptr;
        }

        result = ma_sound_init_from_data_source(engine, &audio->mDecoder->ds, 0, NULL, audio->mSound.get());

        if (result != MA_SUCCESS) {
            GOLIAS_LOG_ERROR("Failed to initialize audio sound for file: %s", path.data());
            return nullptr;
        }

        // TODO: check before enabling spatialization
        ma_sound_set_spatialization_enabled(audio->mSound.get(), MA_TRUE);

        String extension = std::filesystem::path(path.data()).extension().string();

        GOLIAS_LOG_INFO("Audio loaded successfully from file: %s\n"
                        "  Channels: %u\n"
                        "  Sample Rate: %u\n"
                        "  Format: %d | Description: %s\n"
                        "  File Extension: %s",
                        path.data(),
                        audio->mDecoder->outputChannels,
                        audio->mDecoder->outputSampleRate,
                        static_cast<int>(audio->mDecoder->outputFormat),
                        ma_get_format_name(audio->mDecoder->outputFormat),
                        extension.c_str());


        return audio;
    }


    void Audio::Play(bool loop) {
        if (mSound) {
            ma_sound_stop(mSound.get());
            ma_sound_seek_to_pcm_frame(mSound.get(), 0);
            ma_sound_set_looping(mSound.get(), loop ? MA_TRUE : MA_FALSE);
            ma_sound_start(mSound.get());
        }
    }

    void Audio::Stop() {
        if (mSound) {
            ma_sound_stop(mSound.get());
        }
    }

    float Audio::GetVolume() const {
        if (mSound) {
            float volume = ma_sound_get_volume(mSound.get());
            return volume;
        }

        return 0.0f;
    }

    void Audio::SetVolume(float volume) {
        if (mSound) {
            ma_sound_set_volume(mSound.get(), volume);
        }
    }


    void Audio::SetPosition(float x, float y, float z) {
        if (mSound) {
            ma_sound_set_position(mSound.get(), x, y, z);
        }
    }

    bool Audio::IsPlaying() const {
        if (mSound) {
            return ma_sound_is_playing(mSound.get()) == MA_TRUE;
        }

        return false;
    }
} // namespace golias
