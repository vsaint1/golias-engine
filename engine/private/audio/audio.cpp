#include "audio/audio.h"
#include "audio/audio_buffer.h"

#include "core/engine.h"
#include <SDL3/SDL_stdinc.h>
#include <spdlog/spdlog.h>

#define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (float-32bit)
#define AUDIO_DEVICE_CHANNELS  2                // Device output channels: stereo

namespace golias {

    static rAudioBuffer* LoadAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage) {
        rAudioBuffer* audioBuffer = new rAudioBuffer();

        if (sizeInFrames > 0) {
            audioBuffer->data = (unsigned char*)SDL_calloc(sizeInFrames * channels * ma_get_bytes_per_sample(format), 1);
        }

        audioBuffer->format = format;
        audioBuffer->channels = channels;
        audioBuffer->sampleRate = sampleRate;

        auto& audioManager = Engine::GetInstance().GetAudioManager();
        ma_data_converter_config converterConfig = ma_data_converter_config_init(
            format, AUDIO_DEVICE_FORMAT,
            channels, AUDIO_DEVICE_CHANNELS,
            sampleRate, audioManager.GetDevice()->sampleRate
        );
        converterConfig.allowDynamicSampleRate = true;

        ma_result result = ma_data_converter_init(&converterConfig, nullptr, &audioBuffer->converter);
        if (result != MA_SUCCESS) {
            spdlog::error("LoadAudioBuffer: Failed to create data conversion pipeline");
            SDL_free(audioBuffer->data);
            delete audioBuffer;
            return nullptr;
        }

        audioBuffer->volume = 1.0f;
        audioBuffer->pitch = 1.0f;
        audioBuffer->pan = 0.5f;

        audioBuffer->playing = false;
        audioBuffer->paused = false;
        audioBuffer->looping = false;
        audioBuffer->usage = usage;
        audioBuffer->frameCursorPos = 0;
        audioBuffer->sizeInFrames = sizeInFrames;

        audioBuffer->isSubBufferProcessed[0] = true;
        audioBuffer->isSubBufferProcessed[1] = true;

        audioBuffer->spatializer = nullptr;
        audioBuffer->spatializationEnabled = false;
        audioBuffer->positionX = 0.0f;
        audioBuffer->positionY = 0.0f;
        audioBuffer->positionZ = 0.0f;

        audioManager.TrackAudioBuffer(audioBuffer);

        return audioBuffer;
    }

    static void UnloadAudioBuffer(rAudioBuffer* audioBuffer) {
        if (audioBuffer != nullptr) {
            auto& audioManager = Engine::GetInstance().GetAudioManager();
            audioManager.UntrackAudioBuffer(audioBuffer);

            
            if (audioBuffer->spatializer != nullptr) {
                ma_spatializer_uninit(audioBuffer->spatializer, nullptr);
                delete audioBuffer->spatializer;
            }

            ma_data_converter_uninit(&audioBuffer->converter, nullptr);
            SDL_free(audioBuffer->data);
            delete audioBuffer;
        }
    }

    Audio::~Audio() {
        if (buffer) {
            UnloadAudioBuffer(buffer);
            buffer = nullptr;
        }
    }

    void Audio::Play(bool loop) {
        if (buffer != nullptr) {
            buffer->looping = loop;
            buffer->playing = true;
            buffer->paused = false;
            buffer->frameCursorPos = 0;
        }
    }

    void Audio::Stop() {
        if (buffer != nullptr && buffer->playing) {
            buffer->playing = false;
            buffer->paused = false;
            buffer->frameCursorPos = 0;
            buffer->framesProcessed = 0;
            buffer->isSubBufferProcessed[0] = true;
            buffer->isSubBufferProcessed[1] = true;
        }
    }

    void Audio::Pause() {
        if (buffer != nullptr) {
            buffer->paused = true;
        }
    }

    void Audio::Resume() {
        if (buffer != nullptr) {
            buffer->paused = false;
        }
    }

    void Audio::SetVolume(float value) {
        if (buffer != nullptr) {
            buffer->volume = value;
        }
    }

    float Audio::GetVolume() const {
        return buffer ? buffer->volume : 0.0f;
    }

    void Audio::SetPitch(float pitch) {
        if (buffer != nullptr && pitch > 0.0f) {
            
            auto& audioManager = Engine::GetInstance().GetAudioManager();
            ma_uint32 outputSampleRate = (ma_uint32)((float)audioManager.GetDevice()->sampleRate / pitch);
            ma_data_converter_set_rate(&buffer->converter, buffer->sampleRate, outputSampleRate);
            buffer->pitch = pitch;
        }
    }

    float Audio::GetPitch() const {
        return buffer ? buffer->pitch : 1.0f;
    }

    void Audio::SetPan(float pan) {
        if (buffer != nullptr) {
            if (pan < 0.0f) pan = 0.0f;
            else if (pan > 1.0f) pan = 1.0f;
            buffer->pan = pan;
        }
    }

    float Audio::GetPan() const {
        return buffer ? buffer->pan : 0.5f;
    }

    bool Audio::IsPlaying() const {
        return buffer && buffer->playing && !buffer->paused;
    }

    void Audio::SetPosition(float x, float y, float z) {
        if (buffer != nullptr) {
            buffer->positionX = x;
            buffer->positionY = y;
            buffer->positionZ = z;

            if (buffer->spatializationEnabled && buffer->spatializer == nullptr) {
                buffer->spatializer = new ma_spatializer();
                auto& audioManager = Engine::GetInstance().GetAudioManager();
                
                ma_spatializer_config spatializerConfig = ma_spatializer_config_init(buffer->channels, ma_attenuation_model_inverse);
                ma_result result = ma_spatializer_init(&spatializerConfig, nullptr, buffer->spatializer);
                if (result != MA_SUCCESS) {
                    delete buffer->spatializer;
                    buffer->spatializer = nullptr;
                    spdlog::error("Failed to initialize spatializer");
                }
            }

            if (buffer->spatializer != nullptr) {
                ma_spatializer_set_position(buffer->spatializer, x, y, z);
            }
        }
    }

    void Audio::SetSpatialization(bool enabled) {
        if (buffer != nullptr) {
            buffer->spatializationEnabled = enabled;
            
            if (enabled && buffer->spatializer == nullptr) {
                SetPosition(buffer->positionX, buffer->positionY, buffer->positionZ);
            }

            else if (!enabled && buffer->spatializer != nullptr) {
                ma_spatializer_uninit(buffer->spatializer, nullptr);
                delete buffer->spatializer;
                buffer->spatializer = nullptr;
            }
        }
    }

    std::shared_ptr<Audio> Audio::Load(const std::string_view pPath) {
        auto fileData = Engine::GetInstance().GetFileSystem().LoadAssetFile(pPath);
        if (fileData.empty()) {
            spdlog::error("Audio::Load Failed to load audio file: {}", pPath);
            return nullptr;
        }

        auto audio = std::make_shared<Audio>();
        audio->data = std::move(fileData);

        ma_decoder decoder;
        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_s16, 0, 0);
        
        ma_result result = ma_decoder_init_memory(audio->data.data(), audio->data.size(), &decoderConfig, &decoder);
        if (result != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to initialize decoder for: {}", pPath);
            return nullptr;
        }

        ma_uint32 sampleRate = decoder.outputSampleRate;
        ma_uint32 channels = decoder.outputChannels;
        ma_format format = decoder.outputFormat;
        
        ma_uint64 frameCountIn;
        result = ma_decoder_get_length_in_pcm_frames(&decoder, &frameCountIn);
        if (result != MA_SUCCESS) {
            spdlog::error("Audio::Load Failed to get frame count for: {}", pPath);
            ma_decoder_uninit(&decoder);
            return nullptr;
        }

        void* pcmData = SDL_malloc((size_t)frameCountIn * channels * ma_get_bytes_per_sample(format));
        if (pcmData == nullptr) {
            spdlog::error("Audio::Load Failed to allocate memory for PCM data");
            ma_decoder_uninit(&decoder);
            return nullptr;
        }

        ma_uint64 framesRead;
        result = ma_decoder_read_pcm_frames(&decoder, pcmData, frameCountIn, &framesRead);
        ma_decoder_uninit(&decoder);

        if (result != MA_SUCCESS || framesRead == 0) {
            spdlog::error("Audio::Load Failed to read PCM frames from: {}", pPath);
            SDL_free(pcmData);
            return nullptr;
        }

        auto& audioManager = Engine::GetInstance().GetAudioManager();
        ma_uint32 frameCountOut = (ma_uint32)ma_convert_frames(
            nullptr, 0, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, audioManager.GetDevice()->sampleRate,
            nullptr, (ma_uint32)framesRead, format, channels, sampleRate
        );

        if (frameCountOut == 0) {
            spdlog::error("Audio::Load Failed to calculate output frame count");
            SDL_free(pcmData);
            return nullptr;
        }

        audio->buffer = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, audioManager.GetDevice()->sampleRate, frameCountOut, AUDIO_BUFFER_USAGE_STATIC);
        if (audio->buffer == nullptr) {
            spdlog::error("Audio::Load Failed to create audio buffer");
            SDL_free(pcmData);
            return nullptr;
        }

        frameCountOut = (ma_uint32)ma_convert_frames(
            audio->buffer->data, frameCountOut, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, audioManager.GetDevice()->sampleRate,
            pcmData, (ma_uint32)framesRead, format, channels, sampleRate
        );

        SDL_free(pcmData);

        if (frameCountOut == 0) {
            spdlog::error("Audio::Load Failed to convert audio frames");
            UnloadAudioBuffer(audio->buffer);
            audio->buffer = nullptr;
            return nullptr;
        }

        spdlog::info("Audio::Load Successfully loaded audio file: {}", pPath);
        spdlog::info("    > Sample rate:   {}", sampleRate);
        spdlog::info("    > Channels:      {}", channels);
        spdlog::info("    > Frame count:   {}", framesRead);

        return audio;
    }

} // namespace golias
